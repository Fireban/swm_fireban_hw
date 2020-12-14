using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Drawing;
using System.Drawing.Imaging;

namespace SampleViewer
{
    public class SeekwareDevice
    {
        #region constants
        const int MAX_CAMERAS = 10;
        #endregion

        // Seekware return codes
        public enum SwReturnCode
        {
            None,
            OpenEx,
            BadParam
        };

        // Seekware settings
        public enum SwSettings
        {
            ActiveLUT,
            TempUnits,
            Timeout
        }

        // Seekware temperature units 
        public enum SwTemperatureUnits
        {
            Fahrenheit,
            Celsius,
            Kelvin
        };

        // Seeware display LUTs
        public enum SwDisplayLut
        {
            White,
            Black,
            Iron,
            Cool,
            Amber,
            Indigo,
            Tyrian,
            Glory,
            Envy,
            NewWhite,
            NewBlack,
            Spectra,
            Prism,
            NewTyrian,
            NewIron,
            NewAmber,
            Hi,
            HiLo
        };

        #region private variables
        SeekwareStruct mSeekwareStruct;  // the local copy of the Seekware struct after it has been marshalled in from unmanaged memory
        IntPtr mHandle; // the Seekware device handle, it is a pointer to the seekware struct in unmanaged memory
        //private object paletteHandle;
        #endregion

        #region public properties
        public bool IsOpen { get; private set; }

        /// <summary>
        /// Gets the firmware version as a string
        /// </summary>
        public string FirmwareVersion
        {
            get 
            {
                return string.Format("{0}.{1}", mSeekwareStruct.FirmwareVersionMajor, mSeekwareStruct.FirmwareVersionMinor);
            }
        }

        /// <summary>
        /// Gets the serial number from the Seekware struct
        /// </summary>
        public string SerialNumber
        {
            get
            {
                return mSeekwareStruct.SerialNumber;
            }
        }

        /// <summary>
        /// Gets the width of images from the Seekware camera
        /// </summary>
        public int ImageWidth
        {
            get
            {
                return mSeekwareStruct.FrameWidth;
            }
        }

        /// <summary>
        /// Gets the height of images from the Seekware camera
        /// </summary>
        public int ImageHeight
        {
            get
            {
                return mSeekwareStruct.FrameHeight;
            }
        }

        /// <summary>
        /// Gets or sets the temperature units setting of the camera
        /// </summary>
        public SwTemperatureUnits TemperatureUnits 
        {
            get
            {
                int tempUnits = 0;
                if (IsOpen)
                {
                    Seekware_GetSetting(mHandle, SwSettings.TempUnits, out tempUnits);
                }
                return (SwTemperatureUnits)tempUnits;
            }
            set
            {
                if (IsOpen)
                {
                    Seekware_SetSetting(mHandle, SwSettings.TempUnits, (int)value);
                }
            }
        }

        /// <summary>
        /// Gets or sets the display LUT of the camera
        /// </summary>
        public SwDisplayLut DisplayLUT
        {
            get
            {
                int lut = 0;
                if (IsOpen)
                {
                    Seekware_GetSetting(mHandle, SwSettings.ActiveLUT, out lut);
                }
                return (SwDisplayLut) lut;
            }
            set
            {
                if (IsOpen)
                {
                    Seekware_SetSetting(mHandle, SwSettings.ActiveLUT, (int)value);
                }
            }
        }

        #endregion

        #region constructors
        /// <summary>
        /// Contstructor
        /// </summary>
        /// <param name="sw"></param>
        /// <param name="swPtr"></param>
        public SeekwareDevice(SeekwareStruct sw, IntPtr swPtr)
        {
            mSeekwareStruct = sw;
            mHandle = swPtr;
        }
        #endregion

        #region public methods

        /// <summary>
        /// Gets a list of the current Seekware devices attached to the system
        /// </summary>
        /// <returns></returns>
        public static List<SeekwareDevice> GetDeviceList() 
        {
            List<SeekwareDevice> seekDeviceList = new List<SeekwareDevice>();
            IntPtr[] ptrArray = new IntPtr[10];
            int numFound = 0;

            Seekware_Find(ptrArray, ptrArray.Length, out numFound);

            for (int i = 0; i < numFound; i++)
            {
                var seekwareStruct = (SeekwareStruct)Marshal.PtrToStructure(ptrArray[i], typeof(SeekwareStruct));
                seekDeviceList.Add(new SeekwareDevice(seekwareStruct, ptrArray[i]));
            }

            return seekDeviceList;
        }

        /// <summary>
        /// Gets the Seekware SDK version
        /// </summary>
        /// <returns></returns>
        public static SdkInfoStruct GetSdkInfo()
        {
            SdkInfoStruct sdkInfo = new SdkInfoStruct();
            GCHandle sdkInfoHandle = GCHandle.Alloc(sdkInfo, GCHandleType.Pinned);

            SwReturnCode swReturn = Seekware_GetSdkInfo(ref sdkInfo);

            sdkInfoHandle.Free();
            return sdkInfo;
        }

        /// <summary>
        /// Open the Seekware device
        /// </summary>
        /// <returns></returns>
        public bool Open()
        {
            if (IsOpen == false)
            {
                SwReturnCode retCode = Seekware_Open(mHandle);
                if (retCode == SwReturnCode.None || retCode == SwReturnCode.OpenEx)
                {
                    IsOpen = true;
                }
            }

            return IsOpen;
        }

        /// <summary>
        /// Close the Seekware device
        /// </summary>
        public void Close()
        {
            Seekware_Close(mHandle);
            IsOpen = false;
        }

        /// <summary>
        /// Get a new bitmap and thermography image from the camera
        /// </summary>
        public bool GetImage(out Bitmap bitmap, out float[] thermImage)
        {
            thermImage = null;
            bitmap = null;
            SwReturnCode seekRet = SwReturnCode.BadParam;

            if (IsOpen)
            {
                byte[] displayBuffer = new byte[ImageWidth * ImageHeight * 4];
                thermImage = new float[ImageWidth * ImageHeight];
                GCHandle displayBufferHandle = GCHandle.Alloc(displayBuffer, GCHandleType.Pinned);
                GCHandle thermImageHandle = GCHandle.Alloc(thermImage, GCHandleType.Pinned);

                // get the bitmap and thermography image. The binary image buffer is set to 0(null) because we are not going to use it
                seekRet = Seekware_GetImage(mHandle, IntPtr.Zero, thermImageHandle.AddrOfPinnedObject(), displayBufferHandle.AddrOfPinnedObject());
                if (seekRet == SwReturnCode.None)
                {
                    // Create bitmap from RGBA data
                    bitmap = new Bitmap(ImageWidth, ImageHeight, PixelFormat.Format32bppArgb);
                    if (bitmap == null)
                    {
                        return false;
                    }
                    BitmapData bmData = bitmap.LockBits(new Rectangle(0, 0, bitmap.Width, bitmap.Height), ImageLockMode.WriteOnly, bitmap.PixelFormat);
                    IntPtr pNative = bmData.Scan0;
                    Marshal.Copy(displayBuffer, 0, pNative, (bitmap.Width * bitmap.Height * 4));
                    bitmap.UnlockBits(bmData);
                }

                displayBufferHandle.Free();
                thermImageHandle.Free();
            }

            return (seekRet == SwReturnCode.None);
        }

        #endregion

        #region dll imports

        [DllImport("SeekWare.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern SwReturnCode Seekware_Find(IntPtr[] pswlist, int length, out int numfound);

        [DllImport("SeekWare.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern SwReturnCode Seekware_Open(IntPtr handle);

        [DllImport("SeekWare.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern SwReturnCode Seekware_Close(IntPtr handle);

        [DllImport("SeekWare.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern SwReturnCode Seekware_GetSdkInfo(ref SdkInfoStruct sdkInfo);

        [DllImport("SeekWare.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern SwReturnCode Seekware_GetSetting(IntPtr handle, SwSettings id, out int value);

        [DllImport("SeekWare.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern SwReturnCode Seekware_SetSetting(IntPtr handle, SwSettings id, int value);

        [DllImport("SeekWare.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern SwReturnCode Seekware_GetImage(IntPtr handle, IntPtr binaryImage, IntPtr temperatureImage, IntPtr displayImage);

        [DllImport("SeekWare.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern SwReturnCode Analysis_GetPalette(IntPtr handle, int index, IntPtr palette);

        [DllImport("SeekWare.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern SwReturnCode Analysis_GetThermographyMinMax(IntPtr handle, ref float thermographyData, ref float min, ref float max);

        #endregion // dll imports

    }

    [StructLayout(LayoutKind.Sequential)]
    public struct SeekwareStruct
    {
        // Device information
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 16)]
		public string ModelNumber;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 12)]
		public string SerialNumber;

        public byte FirmwareVersionMajor;
        public byte FirmwareVersionMinor;
        public byte FirmwareVersionBuild;
        public byte FirmwareVersionRevision;
        public UInt16 FrameHeight;
        public UInt16 FrameWidth;

        [MarshalAs(UnmanagedType.LPStr)]
        public string WinDevPath;
        public IntPtr WinDevHandle;

        public UInt32 LastReturnCode;

        public IntPtr SdkPrivate;
        public IntPtr SeekWarePrivate;

        public UInt16 RawFrameHeight;
        public UInt16 RawFrameWidth;
    }


    [StructLayout(LayoutKind.Sequential)]
    public struct SdkInfoStruct
    {
        public byte sdk_version_major; // SDK version number 
        public byte sdk_version_minor;
        public byte sdk_build_major;
        public byte sdk_build_minor;
        public byte lib_version_major; // Library version number 
        public byte lib_version_minor;
        public byte lib_build_major;
        public byte lib_build_minor;
    }
}
