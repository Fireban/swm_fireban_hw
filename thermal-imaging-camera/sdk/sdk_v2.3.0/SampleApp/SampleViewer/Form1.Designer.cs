namespace SampleViewer
{
    partial class Form1
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            this.pictureBox1 = new System.Windows.Forms.PictureBox();
            this.selectCameraComboBox = new System.Windows.Forms.ComboBox();
            this.playButton = new System.Windows.Forms.Button();
            this.stopButton = new System.Windows.Forms.Button();
            this.cameraGroupBox = new System.Windows.Forms.GroupBox();
            this.frameDimTextBox = new System.Windows.Forms.TextBox();
            this.label5 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.firmwareVersionTextBox = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.serialNumberTextBox = new System.Windows.Forms.TextBox();
            this.imageGroupBox = new System.Windows.Forms.GroupBox();
            this.lutComboBox = new System.Windows.Forms.ComboBox();
            this.tempUnitsComboBox = new System.Windows.Forms.ComboBox();
            this.label4 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.fpsTimer = new System.Windows.Forms.Timer(this.components);
            this.statusStrip1 = new System.Windows.Forms.StatusStrip();
            this.toolStripStatusLabel1 = new System.Windows.Forms.ToolStripStatusLabel();
            this.pixelStripStatusLabel = new System.Windows.Forms.ToolStripStatusLabel();
            this.pixelValueStripStatusLabel = new System.Windows.Forms.ToolStripStatusLabel();
            this.errorToolStripStatusLabel = new System.Windows.Forms.ToolStripStatusLabel();
            this.toolStripStatusLabel4 = new System.Windows.Forms.ToolStripStatusLabel();
            this.frameCountStripStatusLabel = new System.Windows.Forms.ToolStripStatusLabel();
            this.toolStripStatusLabel6 = new System.Windows.Forms.ToolStripStatusLabel();
            this.fpsStripStatusLabel = new System.Windows.Forms.ToolStripStatusLabel();
            this.findDevicesTimer = new System.Windows.Forms.Timer(this.components);
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.sdkVersionTextBox = new System.Windows.Forms.TextBox();
            this.label6 = new System.Windows.Forms.Label();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).BeginInit();
            this.cameraGroupBox.SuspendLayout();
            this.imageGroupBox.SuspendLayout();
            this.statusStrip1.SuspendLayout();
            this.groupBox1.SuspendLayout();
            this.SuspendLayout();
            // 
            // pictureBox1
            // 
            this.pictureBox1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.pictureBox1.BackColor = System.Drawing.SystemColors.ControlDark;
            this.pictureBox1.ErrorImage = null;
            this.pictureBox1.Image = global::SampleViewer.Properties.Resources.testPattern;
            this.pictureBox1.InitialImage = null;
            this.pictureBox1.Location = new System.Drawing.Point(290, 12);
            this.pictureBox1.Name = "pictureBox1";
            this.pictureBox1.Size = new System.Drawing.Size(405, 308);
            this.pictureBox1.SizeMode = System.Windows.Forms.PictureBoxSizeMode.Zoom;
            this.pictureBox1.TabIndex = 0;
            this.pictureBox1.TabStop = false;
            this.pictureBox1.MouseMove += new System.Windows.Forms.MouseEventHandler(this.pictureBox1_MouseMove);
            // 
            // selectCameraComboBox
            // 
            this.selectCameraComboBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.selectCameraComboBox.FormattingEnabled = true;
            this.selectCameraComboBox.Location = new System.Drawing.Point(12, 12);
            this.selectCameraComboBox.Name = "selectCameraComboBox";
            this.selectCameraComboBox.Size = new System.Drawing.Size(272, 21);
            this.selectCameraComboBox.TabIndex = 1;
            this.selectCameraComboBox.SelectedIndexChanged += new System.EventHandler(this.selectCameraComboBox_SelectedIndexChanged);
            // 
            // playButton
            // 
            this.playButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.playButton.Location = new System.Drawing.Point(6, 77);
            this.playButton.Name = "playButton";
            this.playButton.Size = new System.Drawing.Size(116, 40);
            this.playButton.TabIndex = 2;
            this.playButton.Text = "Play";
            this.playButton.UseVisualStyleBackColor = true;
            this.playButton.Click += new System.EventHandler(this.playButton_Click);
            // 
            // stopButton
            // 
            this.stopButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.stopButton.Location = new System.Drawing.Point(136, 77);
            this.stopButton.Name = "stopButton";
            this.stopButton.Size = new System.Drawing.Size(116, 40);
            this.stopButton.TabIndex = 3;
            this.stopButton.Text = "Stop";
            this.stopButton.UseVisualStyleBackColor = true;
            this.stopButton.Click += new System.EventHandler(this.stopButton_Click);
            // 
            // cameraGroupBox
            // 
            this.cameraGroupBox.Controls.Add(this.frameDimTextBox);
            this.cameraGroupBox.Controls.Add(this.label5);
            this.cameraGroupBox.Controls.Add(this.label2);
            this.cameraGroupBox.Controls.Add(this.firmwareVersionTextBox);
            this.cameraGroupBox.Controls.Add(this.label1);
            this.cameraGroupBox.Controls.Add(this.serialNumberTextBox);
            this.cameraGroupBox.Location = new System.Drawing.Point(12, 88);
            this.cameraGroupBox.Name = "cameraGroupBox";
            this.cameraGroupBox.Size = new System.Drawing.Size(272, 99);
            this.cameraGroupBox.TabIndex = 5;
            this.cameraGroupBox.TabStop = false;
            this.cameraGroupBox.Text = "Camera";
            // 
            // frameDimTextBox
            // 
            this.frameDimTextBox.Location = new System.Drawing.Point(127, 68);
            this.frameDimTextBox.Name = "frameDimTextBox";
            this.frameDimTextBox.ReadOnly = true;
            this.frameDimTextBox.Size = new System.Drawing.Size(125, 20);
            this.frameDimTextBox.TabIndex = 5;
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(6, 71);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(59, 13);
            this.label5.TabIndex = 4;
            this.label5.Text = "Frame Size";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(6, 48);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(87, 13);
            this.label2.TabIndex = 3;
            this.label2.Text = "Firmware Version";
            // 
            // firmwareVersionTextBox
            // 
            this.firmwareVersionTextBox.Location = new System.Drawing.Point(127, 45);
            this.firmwareVersionTextBox.Name = "firmwareVersionTextBox";
            this.firmwareVersionTextBox.ReadOnly = true;
            this.firmwareVersionTextBox.Size = new System.Drawing.Size(125, 20);
            this.firmwareVersionTextBox.TabIndex = 2;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(6, 27);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(73, 13);
            this.label1.TabIndex = 1;
            this.label1.Text = "Serial Number";
            // 
            // serialNumberTextBox
            // 
            this.serialNumberTextBox.Location = new System.Drawing.Point(127, 24);
            this.serialNumberTextBox.Name = "serialNumberTextBox";
            this.serialNumberTextBox.ReadOnly = true;
            this.serialNumberTextBox.Size = new System.Drawing.Size(125, 20);
            this.serialNumberTextBox.TabIndex = 0;
            // 
            // imageGroupBox
            // 
            this.imageGroupBox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left)));
            this.imageGroupBox.Controls.Add(this.lutComboBox);
            this.imageGroupBox.Controls.Add(this.tempUnitsComboBox);
            this.imageGroupBox.Controls.Add(this.label4);
            this.imageGroupBox.Controls.Add(this.label3);
            this.imageGroupBox.Controls.Add(this.playButton);
            this.imageGroupBox.Controls.Add(this.stopButton);
            this.imageGroupBox.Location = new System.Drawing.Point(12, 193);
            this.imageGroupBox.Name = "imageGroupBox";
            this.imageGroupBox.Size = new System.Drawing.Size(272, 127);
            this.imageGroupBox.TabIndex = 6;
            this.imageGroupBox.TabStop = false;
            this.imageGroupBox.Text = "Image";
            // 
            // lutComboBox
            // 
            this.lutComboBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.lutComboBox.FormattingEnabled = true;
            this.lutComboBox.Location = new System.Drawing.Point(127, 40);
            this.lutComboBox.Name = "lutComboBox";
            this.lutComboBox.Size = new System.Drawing.Size(125, 21);
            this.lutComboBox.TabIndex = 3;
            this.lutComboBox.SelectedIndexChanged += new System.EventHandler(this.lutComboBox_SelectedIndexChanged);
            // 
            // tempUnitsComboBox
            // 
            this.tempUnitsComboBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.tempUnitsComboBox.FormattingEnabled = true;
            this.tempUnitsComboBox.Location = new System.Drawing.Point(127, 17);
            this.tempUnitsComboBox.Name = "tempUnitsComboBox";
            this.tempUnitsComboBox.Size = new System.Drawing.Size(125, 21);
            this.tempUnitsComboBox.TabIndex = 2;
            this.tempUnitsComboBox.SelectedIndexChanged += new System.EventHandler(this.tempUnitsComboBoxSelectedIndexChanged);
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(9, 43);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(65, 13);
            this.label4.TabIndex = 1;
            this.label4.Text = "Display LUT";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(9, 20);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(94, 13);
            this.label3.TabIndex = 0;
            this.label3.Text = "Temperature Units";
            // 
            // fpsTimer
            // 
            this.fpsTimer.Interval = 200;
            this.fpsTimer.Tick += new System.EventHandler(this.fpsTimer_Tick);
            // 
            // statusStrip1
            // 
            this.statusStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripStatusLabel1,
            this.pixelStripStatusLabel,
            this.pixelValueStripStatusLabel,
            this.errorToolStripStatusLabel,
            this.toolStripStatusLabel4,
            this.frameCountStripStatusLabel,
            this.toolStripStatusLabel6,
            this.fpsStripStatusLabel});
            this.statusStrip1.Location = new System.Drawing.Point(0, 333);
            this.statusStrip1.Name = "statusStrip1";
            this.statusStrip1.Size = new System.Drawing.Size(707, 22);
            this.statusStrip1.TabIndex = 18;
            this.statusStrip1.Text = "statusStrip1";
            // 
            // toolStripStatusLabel1
            // 
            this.toolStripStatusLabel1.Name = "toolStripStatusLabel1";
            this.toolStripStatusLabel1.Size = new System.Drawing.Size(31, 17);
            this.toolStripStatusLabel1.Text = "Pixel";
            // 
            // pixelStripStatusLabel
            // 
            this.pixelStripStatusLabel.Name = "pixelStripStatusLabel";
            this.pixelStripStatusLabel.Size = new System.Drawing.Size(63, 17);
            this.pixelStripStatusLabel.Text = "{X=0, Y=0}";
            // 
            // pixelValueStripStatusLabel
            // 
            this.pixelValueStripStatusLabel.Name = "pixelValueStripStatusLabel";
            this.pixelValueStripStatusLabel.Size = new System.Drawing.Size(0, 17);
            // 
            // errorToolStripStatusLabel
            // 
            this.errorToolStripStatusLabel.Name = "errorToolStripStatusLabel";
            this.errorToolStripStatusLabel.Size = new System.Drawing.Size(273, 17);
            this.errorToolStripStatusLabel.Spring = true;
            // 
            // toolStripStatusLabel4
            // 
            this.toolStripStatusLabel4.Name = "toolStripStatusLabel4";
            this.toolStripStatusLabel4.Size = new System.Drawing.Size(273, 17);
            this.toolStripStatusLabel4.Spring = true;
            this.toolStripStatusLabel4.Text = "Frames";
            this.toolStripStatusLabel4.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // frameCountStripStatusLabel
            // 
            this.frameCountStripStatusLabel.Name = "frameCountStripStatusLabel";
            this.frameCountStripStatusLabel.Size = new System.Drawing.Size(13, 17);
            this.frameCountStripStatusLabel.Text = "0";
            // 
            // toolStripStatusLabel6
            // 
            this.toolStripStatusLabel6.Name = "toolStripStatusLabel6";
            this.toolStripStatusLabel6.Size = new System.Drawing.Size(26, 17);
            this.toolStripStatusLabel6.Text = "FPS";
            // 
            // fpsStripStatusLabel
            // 
            this.fpsStripStatusLabel.Name = "fpsStripStatusLabel";
            this.fpsStripStatusLabel.Size = new System.Drawing.Size(13, 17);
            this.fpsStripStatusLabel.Text = "0";
            // 
            // findDevicesTimer
            // 
            this.findDevicesTimer.Interval = 1000;
            this.findDevicesTimer.Tick += new System.EventHandler(this.findDevicesTimer_Tick);
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.sdkVersionTextBox);
            this.groupBox1.Controls.Add(this.label6);
            this.groupBox1.Location = new System.Drawing.Point(12, 40);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(272, 42);
            this.groupBox1.TabIndex = 19;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "SDK";
            // 
            // sdkVersionTextBox
            // 
            this.sdkVersionTextBox.Location = new System.Drawing.Point(127, 13);
            this.sdkVersionTextBox.Name = "sdkVersionTextBox";
            this.sdkVersionTextBox.ReadOnly = true;
            this.sdkVersionTextBox.Size = new System.Drawing.Size(125, 20);
            this.sdkVersionTextBox.TabIndex = 1;
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(9, 16);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(67, 13);
            this.label6.TabIndex = 0;
            this.label6.Text = "SDK Version";
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(707, 355);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.statusStrip1);
            this.Controls.Add(this.imageGroupBox);
            this.Controls.Add(this.cameraGroupBox);
            this.Controls.Add(this.selectCameraComboBox);
            this.Controls.Add(this.pictureBox1);
            this.Name = "Form1";
            this.Text = "Sample Viewer";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.Form1_FormClosing);
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).EndInit();
            this.cameraGroupBox.ResumeLayout(false);
            this.cameraGroupBox.PerformLayout();
            this.imageGroupBox.ResumeLayout(false);
            this.imageGroupBox.PerformLayout();
            this.statusStrip1.ResumeLayout(false);
            this.statusStrip1.PerformLayout();
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.PictureBox pictureBox1;
        private System.Windows.Forms.ComboBox selectCameraComboBox;
        private System.Windows.Forms.Button playButton;
        private System.Windows.Forms.Button stopButton;
        private System.Windows.Forms.GroupBox cameraGroupBox;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.TextBox firmwareVersionTextBox;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox serialNumberTextBox;
        private System.Windows.Forms.GroupBox imageGroupBox;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.ComboBox lutComboBox;
        private System.Windows.Forms.ComboBox tempUnitsComboBox;
        private System.Windows.Forms.Timer fpsTimer;
        private System.Windows.Forms.StatusStrip statusStrip1;
        private System.Windows.Forms.ToolStripStatusLabel toolStripStatusLabel1;
        private System.Windows.Forms.ToolStripStatusLabel pixelStripStatusLabel;
        private System.Windows.Forms.ToolStripStatusLabel pixelValueStripStatusLabel;
        private System.Windows.Forms.ToolStripStatusLabel toolStripStatusLabel4;
        private System.Windows.Forms.ToolStripStatusLabel frameCountStripStatusLabel;
        private System.Windows.Forms.ToolStripStatusLabel toolStripStatusLabel6;
        private System.Windows.Forms.ToolStripStatusLabel fpsStripStatusLabel;
        private System.Windows.Forms.Timer findDevicesTimer;
        private System.Windows.Forms.TextBox frameDimTextBox;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.TextBox sdkVersionTextBox;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.ToolStripStatusLabel errorToolStripStatusLabel;
    }
}

