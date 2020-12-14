using System;
using System.Collections.Generic;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Threading;
using System.Windows.Forms;

namespace SampleViewer
{
    public partial class Form1 : Form
    {

        #region Enums
        /// <summary>
        /// Enumeration for states of the form. These states affect which controls in the form are enabled/disabled
        /// </summary>
        enum ControlStates
        {
            Closed,
            OpenStopped,
            OpenPlaying
        }
        #endregion

        #region private variables

        List<SeekwareDevice> mSeekDeviceList = new List<SeekwareDevice>();
        SeekwareDevice mActiveDevice = null;
        Thread mGrabThread;
        bool mStopImageGrab = true;
        ControlStates mState = ControlStates.Closed;

        // image stats
        float[] mLastThermImage;
        Size mImageSize = new Size();
        Point mMousePixel = new Point();

        // frame counters and timers
        uint mFrameCounter = 0;
        List<DateTime> mFrameTimeList = new List<DateTime>();
        private object mTimerLockObject = new object();
        private object mDeviceListLockObject = new object();

        #endregion

        #region Properties

        /// <summary>
        /// Sets/gets the current state of the form. Setting the state affects if other forms are enabled/disabled
        /// </summary>
        ControlStates State 
        {
            get
            {
                return mState;
            }
            
            set
            {
                mState = value;
                switch (value)
                {
                    case ControlStates.Closed:
                        playButton.Enabled = false;
                        stopButton.Enabled = false;
                        selectCameraComboBox.Enabled = true;
                        break;

                    case ControlStates.OpenPlaying:
                        playButton.Enabled = false;
                        stopButton.Enabled = true;
                        selectCameraComboBox.Enabled = false;
                        break;

                    case ControlStates.OpenStopped:
                        playButton.Enabled = true;
                        stopButton.Enabled = false;
                        selectCameraComboBox.Enabled = true;
                        break;
                }
            }
        }

        #endregion

        /// <summary>
        /// Contructor of the main form
        /// </summary>
        public Form1()
        {
            InitializeComponent();

            SdkInfoStruct sdkInfo = SeekwareDevice.GetSdkInfo();
            sdkVersionTextBox.Text = string.Format("{0}.{1}", sdkInfo.sdk_version_major, sdkInfo.sdk_version_minor);

            // populate device list combo box
            mSeekDeviceList = SeekwareDevice.GetDeviceList();
            foreach (var dev in mSeekDeviceList)
            {
                selectCameraComboBox.Items.Add(dev.SerialNumber);
            }

            // populate temperature units combo box
            tempUnitsComboBox.DataSource = Enum.GetValues(typeof(SeekwareDevice.SwTemperatureUnits));
            tempUnitsComboBox.SelectedItem = SeekwareDevice.SwTemperatureUnits.Celsius;

            // populate the display lut
            lutComboBox.DataSource = Enum.GetValues(typeof(SeekwareDevice.SwDisplayLut));
            lutComboBox.SelectedItem = SeekwareDevice.SwDisplayLut.Spectra;

            // set the initial state and start the find devices timer
            State = ControlStates.Closed;
            findDevicesTimer.Enabled = true;
        }

        #region Private Methods

        /// <summary>
        /// Open a SeekwareDevice.
        /// </summary>
        /// <param name="seekDev"></param>
        private void Open(SeekwareDevice seekDev)
        {
            if (seekDev.Open())
            {
                // setup device whith current settings
                seekDev.TemperatureUnits = (SeekwareDevice.SwTemperatureUnits)tempUnitsComboBox.SelectedItem;
                seekDev.DisplayLUT = (SeekwareDevice.SwDisplayLut) lutComboBox.SelectedItem;

                // get info from device and fill in form items
                mImageSize = new Size(seekDev.ImageWidth, seekDev.ImageHeight);
                serialNumberTextBox.Text = seekDev.SerialNumber;
                firmwareVersionTextBox.Text = seekDev.FirmwareVersion;
                frameDimTextBox.Text = string.Format("Width={0}, Height={1}", mImageSize.Width, mImageSize.Height);
                errorToolStripStatusLabel.Text = "Open";
                // set state
                this.BeginInvoke(new Action(() => State = ControlStates.OpenStopped));
            }
            else
            {
                // Open failed, clear form data
                serialNumberTextBox.Text = string.Empty;
                firmwareVersionTextBox.Text = string.Empty;
                frameDimTextBox.Text = string.Empty;
                errorToolStripStatusLabel.Text = "Open Failed";
                // set state
                this.BeginInvoke(new Action(() => State = ControlStates.Closed));
            }
        }

        /// <summary>
        /// Close a SeekwareDevice
        /// </summary>
        /// <param name="seekDev"></param>
        private void Close(SeekwareDevice seekDev)
        {
            if (seekDev.IsOpen)
            {
                StopLiveCapture();
                seekDev.Close();
            }
            // clear form data
            serialNumberTextBox.Text = string.Empty;
            firmwareVersionTextBox.Text = string.Empty;
            frameDimTextBox.Text = string.Empty;
            errorToolStripStatusLabel.Text = "Closed";
            // set state
            this.BeginInvoke(new Action(() => State = ControlStates.Closed));
        }

        /// <summary>
        /// Creates a new thread to begin continuous image captures
        /// </summary>
        private void StartLiveCapture()
        {
            if (mGrabThread == null)
            {
                mStopImageGrab = false;
                mGrabThread = new Thread(new ThreadStart(LiveCapture));
                mGrabThread.Start();
                // start the update stats timer
                fpsTimer.Start();
            }
        }

        /// <summary>
        /// Stops the thread that runs the continuous image capture
        /// </summary>
        private void StopLiveCapture()
        {
            if (mGrabThread != null)
            {
                // stop the stats timer
                fpsTimer.Stop();

                mStopImageGrab = true; // signal the thread to stop
                mGrabThread.Join();    // wait for the thread to end
                mGrabThread = null;    // clear the thread
            }
        }

        /// <summary>
        /// 
        /// This will coninuously grab images and update the pictureBox image until
        /// signaled to stop
        /// </summary>
        private void LiveCapture()
        {
            this.BeginInvoke(new Action(() => State = ControlStates.OpenPlaying));
            while (!mStopImageGrab)
            {
                if (mActiveDevice == null || !mActiveDevice.IsOpen)
                {
                    break;
                }

                Bitmap bitmap = null;
                float[] thermImage = null;

                if (mActiveDevice.GetImage(out bitmap, out thermImage))
                {
                    mLastThermImage = thermImage;

                    if (bitmap != null)
                    {
                        lock (mTimerLockObject)
                        {
                            mFrameCounter++;
                            mFrameTimeList.Add(DateTime.Now);
                        }
                        pictureBox1.Image = bitmap;
                    }

                    this.BeginInvoke(new Action(() => UpdatePixelValueStrip()));
                }
            }
            this.BeginInvoke(new Action(() => State = ControlStates.OpenStopped));
        }
        
        /// <summary>
        /// Update the temperature value at the mouse location. This is called on a mouse move event and when a
        /// new image is received
        /// </summary>
        private void UpdatePixelValueStrip()
        {
            pixelStripStatusLabel.Text = mMousePixel.ToString();
            if (mLastThermImage != null)
            {
                float v = mLastThermImage[(mMousePixel.Y * mImageSize.Width) + mMousePixel.X];
                pixelValueStripStatusLabel.Text = v.ToString("N3") + " " + (SeekwareDevice.SwTemperatureUnits)tempUnitsComboBox.SelectedItem;
            }
        }
        #endregion

        #region User Control Events

        /// <summary>
        /// Opens the SeekwareDevice selected by the combo box. If a device is already open then close it first
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void selectCameraComboBox_SelectedIndexChanged(object sender, EventArgs e)
        {
            // if a device is currently selected, close it first
            lock (mDeviceListLockObject)
            {
                if (mActiveDevice != null)
                {
                    Close(mActiveDevice);
                }

                if (selectCameraComboBox.SelectedIndex < mSeekDeviceList.Count)
                {
                    // find the device in the deviceList that matches the serial number of the selected item from the combo box
                    mActiveDevice = mSeekDeviceList.Where(x => x.SerialNumber == (string)selectCameraComboBox.SelectedItem).FirstOrDefault();
                    // open the device if dound
                    if (mActiveDevice != null)
                    {
                        Open(mActiveDevice);
                    }
                }
            }
        }

        /// <summary>
        /// Start live streaming of images.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void playButton_Click(object sender, EventArgs e)
        {
            StartLiveCapture();
        }

        /// <summary>
        /// Stop live streaming of images
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void stopButton_Click(object sender, EventArgs e)
        {
            StopLiveCapture();
        }

        /// <summary>
        /// Sets the temperature units for a SeekwareDevice 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void tempUnitsComboBoxSelectedIndexChanged(object sender, EventArgs e)
        {
            if (mActiveDevice != null && mActiveDevice.IsOpen)
            {
                mActiveDevice.TemperatureUnits = (SeekwareDevice.SwTemperatureUnits)tempUnitsComboBox.SelectedItem;
            }
        }

        /// <summary>
        /// Sets the display LUT for a SeekwareDevice
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void lutComboBox_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (mActiveDevice != null && mActiveDevice.IsOpen)
            {
                mActiveDevice.DisplayLUT = (SeekwareDevice.SwDisplayLut) lutComboBox.SelectedItem;
            }
        }

        /// <summary>
        /// Updates the mouse image pixel position in the picturebox.
        /// Then it updates the value of the pixel shown in the status strip
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void pictureBox1_MouseMove(object sender, MouseEventArgs e)
        {
            mMousePixel.X = (e.X * mImageSize.Width) / pictureBox1.Width;
            mMousePixel.Y = (e.Y * mImageSize.Height) / pictureBox1.Height;
            UpdatePixelValueStrip();
        }

        /// <summary>
        /// Calculates and displays the frames per second while in live streaming mode
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void fpsTimer_Tick(object sender, EventArgs e)
        {
            if (Monitor.TryEnter(mTimerLockObject))
            {
                try
                {
                    if (mActiveDevice == null || !mActiveDevice.IsOpen)
                    {
                        return;
                    }

                    // remove all frame times from the list that are older than 2 seconds ago
                    mFrameTimeList.RemoveAll(x => (DateTime.Now - x) > TimeSpan.FromMilliseconds(2000));

                    // calculate the average period between frame captures
                    if (mFrameTimeList.Count > 1)
                    {
                        double averageTime = 0;
                        DateTime lastTime = mFrameTimeList[0];
                        for (int i = 1; i < mFrameTimeList.Count; i++)
                        {
                            averageTime += (mFrameTimeList[i] - lastTime).Milliseconds;
                            lastTime = mFrameTimeList[i];
                        }
                        averageTime /= mFrameTimeList.Count - 1;
                        fpsStripStatusLabel.Text = (1000 / averageTime).ToString("N1"); 
                    }

                    frameCountStripStatusLabel.Text = mFrameCounter.ToString(); 

                }
                finally
                {
                    Monitor.Exit(mTimerLockObject);
                }
            }
        }

        /// <summary>
        /// Get connected SeewareDevice list on the timer tick
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void findDevicesTimer_Tick(object sender, EventArgs e)
        {
            findDevicesTimer.Enabled = false;

            // gets a list of attached devices
            List<SeekwareDevice> newDevices = SeekwareDevice.GetDeviceList();

            List<SeekwareDevice> addedDevices = new List<SeekwareDevice>();
            List<SeekwareDevice> removedDevices = new List<SeekwareDevice>();

            lock (mDeviceListLockObject)
            {
                // make a list of new devices that are not in our current list
                foreach (var dev in newDevices)
                {
                    if (mSeekDeviceList.FindIndex(x => x.SerialNumber == dev.SerialNumber) < 0)
                    {
                        addedDevices.Add(dev);
                    }
                }

                // make a list of devices that have been removed from our current list
                foreach (var dev in mSeekDeviceList)
                {
                    if (newDevices.FindIndex(x => x.SerialNumber == dev.SerialNumber) < 0)
                    {
                        removedDevices.Add(dev);
                    }
                }

                // add any new devices to the mSeekDeviceList and the device combobox
                foreach (var dev in addedDevices)
                {
                    mSeekDeviceList.Add(dev);
                    selectCameraComboBox.Items.Add(dev.SerialNumber);
                }

                // remove any devices that are no longer attached from the mSeekDeviceList and the device combobox
                foreach (var remove in removedDevices)
                {
                    if (mActiveDevice != null && mActiveDevice.SerialNumber == remove.SerialNumber)
                    {
                        if (mActiveDevice.IsOpen)
                        {
                            Close(mActiveDevice);
                        }
                    }

                    mSeekDeviceList.Remove(remove);
                    selectCameraComboBox.Items.Remove(remove.SerialNumber);
                }
            }
            findDevicesTimer.Enabled = true;
        }

        /// <summary>
        /// On the main form closing event, stop live streaming if it is on.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            StopLiveCapture();
            if (mActiveDevice != null && mActiveDevice.IsOpen)
            {
                Close(mActiveDevice);
            }
        }
        #endregion
    }
}
