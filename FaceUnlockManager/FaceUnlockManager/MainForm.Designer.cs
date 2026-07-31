namespace FaceUnlockManager
{
    partial class MainForm
    {
        /// <summary>
        ///  Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        ///  Clean up any resources being used.
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
        ///  Required method for Designer support - do not modify
        ///  the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent() {
            components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MainForm));
            LableTitle = new Label();
            labelSubTitle = new Label();
            BtnRegister = new Button();
            imageList1 = new ImageList(components);
            BtnPassword = new Button();
            BtnRemove = new Button();
            PanelPassword = new Panel();
            LabelAttemptWarn = new Label();
            imageWarn = new ImageList(components);
            BtnPassPnlExt = new Button();
            BtnPassPnlConf = new Button();
            PasswordBox = new TextBox();
            LabelAccName = new Label();
            LableTitlePassPnl = new Label();
            tableLayoutPanel1 = new TableLayoutPanel();
            labelRegistered = new Label();
            labelActive = new Label();
            PicBoxActive = new PictureBox();
            PicBoxFaceReg = new PictureBox();
            PanelStatus = new Panel();
            pictureBox2 = new PictureBox();
            label1 = new Label();
            PanelToast = new TableLayoutPanel();
            ToastIcon = new PictureBox();
            ToastText = new Label();
            PanelMainUI = new Panel();
            AddAltFace = new Button();
            LabelVersion = new Label();
            PanelPassword.SuspendLayout();
            tableLayoutPanel1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)PicBoxActive).BeginInit();
            ((System.ComponentModel.ISupportInitialize)PicBoxFaceReg).BeginInit();
            PanelStatus.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)pictureBox2).BeginInit();
            PanelToast.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)ToastIcon).BeginInit();
            PanelMainUI.SuspendLayout();
            SuspendLayout();
            // 
            // LableTitle
            // 
            LableTitle.AutoSize = true;
            LableTitle.FlatStyle = FlatStyle.Flat;
            LableTitle.Font = new Font("MS Reference Sans Serif", 18F, FontStyle.Bold, GraphicsUnit.Point, 0);
            LableTitle.Location = new Point(158, 26);
            LableTitle.Name = "LableTitle";
            LableTitle.Size = new Size(359, 38);
            LableTitle.TabIndex = 0;
            LableTitle.Text = "Face Unlock Manager";
            // 
            // labelSubTitle
            // 
            labelSubTitle.AutoSize = true;
            labelSubTitle.ForeColor = Color.DimGray;
            labelSubTitle.Location = new Point(196, 67);
            labelSubTitle.Name = "labelSubTitle";
            labelSubTitle.Size = new Size(283, 20);
            labelSubTitle.TabIndex = 1;
            labelSubTitle.Text = "Custom Face Authentication For Windows";
            // 
            // BtnRegister
            // 
            BtnRegister.Cursor = Cursors.Hand;
            BtnRegister.Font = new Font("Segoe UI Semibold", 9F, FontStyle.Bold, GraphicsUnit.Point, 0);
            BtnRegister.ForeColor = Color.FromArgb(64, 64, 64);
            BtnRegister.ImageAlign = ContentAlignment.MiddleLeft;
            BtnRegister.ImageIndex = 0;
            BtnRegister.ImageList = imageList1;
            BtnRegister.Location = new Point(60, 178);
            BtnRegister.Name = "BtnRegister";
            BtnRegister.Padding = new Padding(10, 0, 0, 0);
            BtnRegister.Size = new Size(283, 45);
            BtnRegister.TabIndex = 3;
            BtnRegister.Text = "Register / Update Face";
            BtnRegister.UseVisualStyleBackColor = true;
            BtnRegister.Click += BtnRegister_Click;
            // 
            // imageList1
            // 
            imageList1.ColorDepth = ColorDepth.Depth32Bit;
            imageList1.ImageStream = (ImageListStreamer)resources.GetObject("imageList1.ImageStream");
            imageList1.TransparentColor = Color.Transparent;
            imageList1.Images.SetKeyName(0, "faceid.png");
            imageList1.Images.SetKeyName(1, "disable.png");
            imageList1.Images.SetKeyName(2, "password.png");
            imageList1.Images.SetKeyName(3, "tick.png");
            imageList1.Images.SetKeyName(4, "tick_filled.png");
            imageList1.Images.SetKeyName(5, "cross.png");
            imageList1.Images.SetKeyName(6, "cross_filled.png");
            imageList1.Images.SetKeyName(7, "adduser.png");
            // 
            // BtnPassword
            // 
            BtnPassword.Cursor = Cursors.Hand;
            BtnPassword.Font = new Font("Segoe UI Semibold", 9F, FontStyle.Bold, GraphicsUnit.Point, 0);
            BtnPassword.ForeColor = Color.FromArgb(64, 64, 64);
            BtnPassword.ImageAlign = ContentAlignment.MiddleLeft;
            BtnPassword.ImageIndex = 2;
            BtnPassword.ImageList = imageList1;
            BtnPassword.Location = new Point(60, 229);
            BtnPassword.Name = "BtnPassword";
            BtnPassword.Padding = new Padding(10, 0, 0, 0);
            BtnPassword.Size = new Size(283, 45);
            BtnPassword.TabIndex = 4;
            BtnPassword.Text = "Update Password";
            BtnPassword.UseVisualStyleBackColor = true;
            BtnPassword.Click += BtnPassword_Click;
            // 
            // BtnRemove
            // 
            BtnRemove.Cursor = Cursors.Hand;
            BtnRemove.Font = new Font("Segoe UI Semibold", 9F, FontStyle.Bold, GraphicsUnit.Point, 0);
            BtnRemove.ForeColor = Color.FromArgb(64, 64, 64);
            BtnRemove.ImageAlign = ContentAlignment.MiddleLeft;
            BtnRemove.ImageIndex = 1;
            BtnRemove.ImageList = imageList1;
            BtnRemove.Location = new Point(60, 280);
            BtnRemove.Name = "BtnRemove";
            BtnRemove.Padding = new Padding(10, 0, 0, 0);
            BtnRemove.Size = new Size(283, 45);
            BtnRemove.TabIndex = 5;
            BtnRemove.Text = "Disable Face Unlock";
            BtnRemove.UseVisualStyleBackColor = true;
            BtnRemove.Click += BtnRemove_Click;
            // 
            // PanelPassword
            // 
            PanelPassword.BackColor = Color.White;
            PanelPassword.Controls.Add(LabelAttemptWarn);
            PanelPassword.Controls.Add(BtnPassPnlExt);
            PanelPassword.Controls.Add(BtnPassPnlConf);
            PanelPassword.Controls.Add(PasswordBox);
            PanelPassword.Controls.Add(LabelAccName);
            PanelPassword.Controls.Add(LableTitlePassPnl);
            PanelPassword.Location = new Point(197, 150);
            PanelPassword.Name = "PanelPassword";
            PanelPassword.Size = new Size(280, 200);
            PanelPassword.TabIndex = 6;
            PanelPassword.Visible = false;
            PanelPassword.VisibleChanged += PanelPassword_VisibleChanged;
            // 
            // LabelAttemptWarn
            // 
            LabelAttemptWarn.ForeColor = Color.IndianRed;
            LabelAttemptWarn.ImageAlign = ContentAlignment.MiddleLeft;
            LabelAttemptWarn.ImageIndex = 0;
            LabelAttemptWarn.ImageList = imageWarn;
            LabelAttemptWarn.Location = new Point(45, 173);
            LabelAttemptWarn.Name = "LabelAttemptWarn";
            LabelAttemptWarn.Size = new Size(196, 25);
            LabelAttemptWarn.TabIndex = 6;
            LabelAttemptWarn.Text = "2 Attempt Remaining";
            LabelAttemptWarn.TextAlign = ContentAlignment.MiddleCenter;
            LabelAttemptWarn.Visible = false;
            // 
            // imageWarn
            // 
            imageWarn.ColorDepth = ColorDepth.Depth32Bit;
            imageWarn.ImageStream = (ImageListStreamer)resources.GetObject("imageWarn.ImageStream");
            imageWarn.TransparentColor = Color.Transparent;
            imageWarn.Images.SetKeyName(0, "warn.png");
            // 
            // BtnPassPnlExt
            // 
            BtnPassPnlExt.BackColor = Color.Gainsboro;
            BtnPassPnlExt.FlatAppearance.BorderSize = 0;
            BtnPassPnlExt.FlatStyle = FlatStyle.Flat;
            BtnPassPnlExt.Location = new Point(146, 116);
            BtnPassPnlExt.Name = "BtnPassPnlExt";
            BtnPassPnlExt.Size = new Size(110, 43);
            BtnPassPnlExt.TabIndex = 5;
            BtnPassPnlExt.Text = "Exit";
            BtnPassPnlExt.UseVisualStyleBackColor = false;
            BtnPassPnlExt.Click += BtnPassPnlExt_Click;
            // 
            // BtnPassPnlConf
            // 
            BtnPassPnlConf.BackColor = Color.DodgerBlue;
            BtnPassPnlConf.FlatStyle = FlatStyle.Flat;
            BtnPassPnlConf.Font = new Font("Segoe UI Semibold", 9F, FontStyle.Bold, GraphicsUnit.Point, 0);
            BtnPassPnlConf.ForeColor = Color.WhiteSmoke;
            BtnPassPnlConf.Location = new Point(25, 116);
            BtnPassPnlConf.Name = "BtnPassPnlConf";
            BtnPassPnlConf.Size = new Size(110, 43);
            BtnPassPnlConf.TabIndex = 4;
            BtnPassPnlConf.Text = "Confirm";
            BtnPassPnlConf.UseVisualStyleBackColor = false;
            BtnPassPnlConf.EnabledChanged += BtnPassPnlConf_EnabledChanged;
            BtnPassPnlConf.Click += BtnPassPnlConf_Click;
            // 
            // PasswordBox
            // 
            PasswordBox.Font = new Font("Segoe UI", 10.8F, FontStyle.Regular, GraphicsUnit.Point, 0);
            PasswordBox.Location = new Point(25, 78);
            PasswordBox.MaxLength = 256;
            PasswordBox.Name = "PasswordBox";
            PasswordBox.PlaceholderText = "Password";
            PasswordBox.Size = new Size(231, 31);
            PasswordBox.TabIndex = 3;
            PasswordBox.TextAlign = HorizontalAlignment.Center;
            PasswordBox.UseSystemPasswordChar = true;
            PasswordBox.KeyDown += PasswordBox_KeyDown;
            // 
            // LabelAccName
            // 
            LabelAccName.BackColor = Color.Transparent;
            LabelAccName.FlatStyle = FlatStyle.System;
            LabelAccName.Font = new Font("Segoe UI Semibold", 9.5F, FontStyle.Bold);
            LabelAccName.ForeColor = Color.Teal;
            LabelAccName.Location = new Point(25, 43);
            LabelAccName.Name = "LabelAccName";
            LabelAccName.Size = new Size(225, 27);
            LabelAccName.TabIndex = 2;
            LabelAccName.Text = "{Account Name}";
            LabelAccName.TextAlign = ContentAlignment.MiddleCenter;
            // 
            // LableTitlePassPnl
            // 
            LableTitlePassPnl.AutoSize = true;
            LableTitlePassPnl.Font = new Font("Segoe UI", 12F, FontStyle.Bold);
            LableTitlePassPnl.ForeColor = Color.FromArgb(64, 64, 64);
            LableTitlePassPnl.Location = new Point(39, 15);
            LableTitlePassPnl.Name = "LableTitlePassPnl";
            LableTitlePassPnl.Size = new Size(205, 28);
            LableTitlePassPnl.TabIndex = 0;
            LableTitlePassPnl.Text = "Enter Your Password";
            // 
            // tableLayoutPanel1
            // 
            tableLayoutPanel1.ColumnCount = 2;
            tableLayoutPanel1.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 100F));
            tableLayoutPanel1.ColumnStyles.Add(new ColumnStyle(SizeType.Absolute, 50F));
            tableLayoutPanel1.Controls.Add(labelRegistered, 0, 1);
            tableLayoutPanel1.Controls.Add(labelActive, 0, 0);
            tableLayoutPanel1.Controls.Add(PicBoxActive, 1, 0);
            tableLayoutPanel1.Controls.Add(PicBoxFaceReg, 1, 1);
            tableLayoutPanel1.Location = new Point(16, 36);
            tableLayoutPanel1.Name = "tableLayoutPanel1";
            tableLayoutPanel1.RowCount = 2;
            tableLayoutPanel1.RowStyles.Add(new RowStyle(SizeType.Percent, 50F));
            tableLayoutPanel1.RowStyles.Add(new RowStyle(SizeType.Percent, 50F));
            tableLayoutPanel1.Size = new Size(249, 87);
            tableLayoutPanel1.TabIndex = 7;
            // 
            // labelRegistered
            // 
            labelRegistered.BackColor = Color.Transparent;
            labelRegistered.Font = new Font("Segoe UI Semibold", 10.2F, FontStyle.Bold, GraphicsUnit.Point, 0);
            labelRegistered.ForeColor = Color.FromArgb(64, 64, 64);
            labelRegistered.Location = new Point(3, 43);
            labelRegistered.Name = "labelRegistered";
            labelRegistered.Size = new Size(160, 42);
            labelRegistered.TabIndex = 9;
            labelRegistered.Text = "Face Registered";
            labelRegistered.TextAlign = ContentAlignment.MiddleLeft;
            // 
            // labelActive
            // 
            labelActive.BackColor = Color.Transparent;
            labelActive.FlatStyle = FlatStyle.Flat;
            labelActive.Font = new Font("Segoe UI Semibold", 10.2F, FontStyle.Bold, GraphicsUnit.Point, 0);
            labelActive.ForeColor = Color.FromArgb(64, 64, 64);
            labelActive.Location = new Point(3, 0);
            labelActive.Name = "labelActive";
            labelActive.Size = new Size(160, 42);
            labelActive.TabIndex = 0;
            labelActive.Text = "Active";
            labelActive.TextAlign = ContentAlignment.MiddleLeft;
            // 
            // PicBoxActive
            // 
            PicBoxActive.BackgroundImageLayout = ImageLayout.None;
            PicBoxActive.Image = Properties.Resources.tick;
            PicBoxActive.Location = new Point(202, 3);
            PicBoxActive.Name = "PicBoxActive";
            PicBoxActive.Size = new Size(44, 37);
            PicBoxActive.SizeMode = PictureBoxSizeMode.Zoom;
            PicBoxActive.TabIndex = 10;
            PicBoxActive.TabStop = false;
            // 
            // PicBoxFaceReg
            // 
            PicBoxFaceReg.BackgroundImageLayout = ImageLayout.None;
            PicBoxFaceReg.Image = Properties.Resources.tick;
            PicBoxFaceReg.Location = new Point(202, 46);
            PicBoxFaceReg.Name = "PicBoxFaceReg";
            PicBoxFaceReg.Size = new Size(44, 38);
            PicBoxFaceReg.SizeMode = PictureBoxSizeMode.Zoom;
            PicBoxFaceReg.TabIndex = 11;
            PicBoxFaceReg.TabStop = false;
            // 
            // PanelStatus
            // 
            PanelStatus.BackColor = Color.White;
            PanelStatus.Controls.Add(pictureBox2);
            PanelStatus.Controls.Add(label1);
            PanelStatus.Controls.Add(tableLayoutPanel1);
            PanelStatus.Location = new Point(60, 18);
            PanelStatus.Name = "PanelStatus";
            PanelStatus.Size = new Size(283, 136);
            PanelStatus.TabIndex = 12;
            // 
            // pictureBox2
            // 
            pictureBox2.BackColor = Color.Gainsboro;
            pictureBox2.Location = new Point(16, 79);
            pictureBox2.Name = "pictureBox2";
            pictureBox2.Size = new Size(249, 1);
            pictureBox2.TabIndex = 12;
            pictureBox2.TabStop = false;
            // 
            // label1
            // 
            label1.BackColor = Color.Transparent;
            label1.Font = new Font("Segoe UI", 12F, FontStyle.Bold, GraphicsUnit.Point, 0);
            label1.ForeColor = Color.FromArgb(64, 64, 64);
            label1.Location = new Point(104, 0);
            label1.Name = "label1";
            label1.Size = new Size(74, 36);
            label1.TabIndex = 13;
            label1.Text = "Status";
            label1.TextAlign = ContentAlignment.MiddleCenter;
            // 
            // PanelToast
            // 
            PanelToast.BackColor = Color.White;
            PanelToast.ColumnCount = 2;
            PanelToast.ColumnStyles.Add(new ColumnStyle(SizeType.Absolute, 35F));
            PanelToast.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 100F));
            PanelToast.Controls.Add(ToastIcon, 0, 0);
            PanelToast.Controls.Add(ToastText, 1, 0);
            PanelToast.Location = new Point(10, 20);
            PanelToast.Name = "PanelToast";
            PanelToast.Padding = new Padding(5);
            PanelToast.RowCount = 1;
            PanelToast.RowStyles.Add(new RowStyle(SizeType.Percent, 100F));
            PanelToast.Size = new Size(250, 50);
            PanelToast.TabIndex = 13;
            PanelToast.Visible = false;
            // 
            // ToastIcon
            // 
            ToastIcon.Image = Properties.Resources.tick_filled;
            ToastIcon.Location = new Point(8, 8);
            ToastIcon.Name = "ToastIcon";
            ToastIcon.Size = new Size(29, 34);
            ToastIcon.SizeMode = PictureBoxSizeMode.Zoom;
            ToastIcon.TabIndex = 0;
            ToastIcon.TabStop = false;
            // 
            // ToastText
            // 
            ToastText.Font = new Font("Segoe UI Semibold", 9F, FontStyle.Bold, GraphicsUnit.Point, 0);
            ToastText.ForeColor = Color.FromArgb(64, 64, 64);
            ToastText.Location = new Point(43, 5);
            ToastText.Name = "ToastText";
            ToastText.Size = new Size(199, 40);
            ToastText.TabIndex = 1;
            ToastText.Text = "Password Registered";
            ToastText.TextAlign = ContentAlignment.MiddleLeft;
            // 
            // PanelMainUI
            // 
            PanelMainUI.Controls.Add(AddAltFace);
            PanelMainUI.Controls.Add(PanelStatus);
            PanelMainUI.Controls.Add(BtnRegister);
            PanelMainUI.Controls.Add(BtnRemove);
            PanelMainUI.Controls.Add(BtnPassword);
            PanelMainUI.Location = new Point(136, 90);
            PanelMainUI.Name = "PanelMainUI";
            PanelMainUI.Size = new Size(403, 347);
            PanelMainUI.TabIndex = 14;
            // 
            // AddAltFace
            // 
            AddAltFace.BackColor = Color.White;
            AddAltFace.Enabled = false;
            AddAltFace.ImageKey = "adduser.png";
            AddAltFace.ImageList = imageList1;
            AddAltFace.Location = new Point(349, 178);
            AddAltFace.Name = "AddAltFace";
            AddAltFace.Size = new Size(45, 45);
            AddAltFace.TabIndex = 13;
            AddAltFace.UseVisualStyleBackColor = false;
            AddAltFace.Click += AddAltFace_Click;
            // 
            // LabelVersion
            // 
            LabelVersion.Anchor = AnchorStyles.Bottom | AnchorStyles.Left;
            LabelVersion.AutoSize = true;
            LabelVersion.Location = new Point(10, 454);
            LabelVersion.Name = "LabelVersion";
            LabelVersion.Size = new Size(94, 20);
            LabelVersion.TabIndex = 15;
            LabelVersion.Text = "Version: 0.0.0";
            // 
            // MainForm
            // 
            AutoScaleDimensions = new SizeF(8F, 20F);
            AutoScaleMode = AutoScaleMode.Font;
            BackColor = Color.WhiteSmoke;
            ClientSize = new Size(674, 483);
            Controls.Add(LabelVersion);
            Controls.Add(PanelToast);
            Controls.Add(PanelPassword);
            Controls.Add(labelSubTitle);
            Controls.Add(LableTitle);
            Controls.Add(PanelMainUI);
            FormBorderStyle = FormBorderStyle.FixedSingle;
            Icon = (Icon)resources.GetObject("$this.Icon");
            MaximizeBox = false;
            Name = "MainForm";
            StartPosition = FormStartPosition.CenterScreen;
            Text = "Face Unlock Manager";
            PanelPassword.ResumeLayout(false);
            PanelPassword.PerformLayout();
            tableLayoutPanel1.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)PicBoxActive).EndInit();
            ((System.ComponentModel.ISupportInitialize)PicBoxFaceReg).EndInit();
            PanelStatus.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)pictureBox2).EndInit();
            PanelToast.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)ToastIcon).EndInit();
            PanelMainUI.ResumeLayout(false);
            ResumeLayout(false);
            PerformLayout();
        }

        #endregion

        private Label LableTitle;
        private Label labelSubTitle;
        private Button BtnRegister;
        private Button BtnPassword;
        private Button BtnRemove;
        private Panel PanelPassword;
        private Label LableTitlePassPnl;
        private Label LabelAccName;
        private TextBox PasswordBox;
        private Button BtnPassPnlExt;
        private Button BtnPassPnlConf;
        private Label LabelAttemptWarn;
        private TableLayoutPanel tableLayoutPanel1;
        private Label labelActive;
        private Label labelRegistered;
        private PictureBox PicBoxActive;
        private PictureBox PicBoxFaceReg;
        private ImageList imageList1;
        private Panel PanelStatus;
        private Label label1;
        private PictureBox pictureBox2;
        private ImageList imageWarn;
        private TableLayoutPanel PanelToast;
        private PictureBox ToastIcon;
        private Label ToastText;
        private Panel PanelMainUI;
        private Label LabelVersion;
        private Button AddAltFace;
    }
}
