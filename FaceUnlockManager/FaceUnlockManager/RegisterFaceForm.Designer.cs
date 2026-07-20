namespace FaceUnlockManager {
    partial class RegisterFaceForm {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing) {
            if (disposing && (components != null)) {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent() {
            components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(RegisterFaceForm));
            LableTitle = new Label();
            LableInstruct = new Label();
            PanelCamera = new Panel();
            BtnCancel = new Button();
            EmbedTimer = new System.Windows.Forms.Timer(components);
            MonitorTimer = new System.Windows.Forms.Timer(components);
            BtnRegister = new Button();
            label1 = new Label();
            SuspendLayout();
            // 
            // LableTitle
            // 
            LableTitle.AutoSize = true;
            LableTitle.FlatStyle = FlatStyle.Flat;
            LableTitle.Font = new Font("MS Reference Sans Serif", 18F, FontStyle.Bold);
            LableTitle.ForeColor = SystemColors.ControlText;
            LableTitle.Location = new Point(34, 23);
            LableTitle.Name = "LableTitle";
            LableTitle.Size = new Size(316, 38);
            LableTitle.TabIndex = 1;
            LableTitle.Text = "Register New Face";
            // 
            // LableInstruct
            // 
            LableInstruct.AutoSize = true;
            LableInstruct.Font = new Font("Segoe UI Semibold", 12F, FontStyle.Bold, GraphicsUnit.Point, 0);
            LableInstruct.ForeColor = Color.FromArgb(64, 64, 64);
            LableInstruct.Location = new Point(58, 111);
            LableInstruct.Name = "LableInstruct";
            LableInstruct.Size = new Size(266, 140);
            LableInstruct.TabIndex = 2;
            LableInstruct.Text = "Look directly at the camera.\r\n\r\nCenter your face in preview.\r\n\r\nHold still for a few seconds.";
            // 
            // PanelCamera
            // 
            PanelCamera.BackColor = Color.White;
            PanelCamera.BackgroundImage = Properties.Resources.faceidwhitesmoke;
            PanelCamera.BackgroundImageLayout = ImageLayout.Stretch;
            PanelCamera.Location = new Point(381, 23);
            PanelCamera.Name = "PanelCamera";
            PanelCamera.Size = new Size(400, 400);
            PanelCamera.TabIndex = 3;
            // 
            // BtnCancel
            // 
            BtnCancel.BackColor = Color.Firebrick;
            BtnCancel.FlatAppearance.BorderSize = 0;
            BtnCancel.FlatStyle = FlatStyle.Flat;
            BtnCancel.ForeColor = Color.WhiteSmoke;
            BtnCancel.Location = new Point(224, 313);
            BtnCancel.Name = "BtnCancel";
            BtnCancel.Size = new Size(100, 43);
            BtnCancel.TabIndex = 0;
            BtnCancel.Text = "Cancel";
            BtnCancel.UseVisualStyleBackColor = false;
            BtnCancel.Click += BtnCancel_Click;
            // 
            // EmbedTimer
            // 
            EmbedTimer.Interval = 50;
            EmbedTimer.Tick += EmbedTimer_Tick;
            // 
            // BtnRegister
            // 
            BtnRegister.BackColor = Color.SteelBlue;
            BtnRegister.FlatAppearance.BorderSize = 0;
            BtnRegister.FlatStyle = FlatStyle.Flat;
            BtnRegister.ForeColor = Color.White;
            BtnRegister.Location = new Point(34, 313);
            BtnRegister.Name = "BtnRegister";
            BtnRegister.RightToLeft = RightToLeft.No;
            BtnRegister.Size = new Size(184, 43);
            BtnRegister.TabIndex = 4;
            BtnRegister.Text = "Start Registration";
            BtnRegister.UseVisualStyleBackColor = false;
            BtnRegister.Click += BtnRegister_Click;
            // 
            // label1
            // 
            label1.AutoSize = true;
            label1.Font = new Font("Segoe UI Black", 12F, FontStyle.Bold, GraphicsUnit.Point, 0);
            label1.ForeColor = SystemColors.HotTrack;
            label1.Location = new Point(34, 111);
            label1.Name = "label1";
            label1.Size = new Size(30, 140);
            label1.TabIndex = 5;
            label1.Text = "1.\r\n\r\n2.\r\n\r\n3.";
            // 
            // RegisterFaceForm
            // 
            AutoScaleDimensions = new SizeF(8F, 20F);
            AutoScaleMode = AutoScaleMode.Font;
            ClientSize = new Size(804, 447);
            ControlBox = false;
            Controls.Add(label1);
            Controls.Add(BtnRegister);
            Controls.Add(BtnCancel);
            Controls.Add(PanelCamera);
            Controls.Add(LableInstruct);
            Controls.Add(LableTitle);
            FormBorderStyle = FormBorderStyle.FixedSingle;
            Icon = (Icon)resources.GetObject("$this.Icon");
            MaximizeBox = false;
            MinimizeBox = false;
            Name = "RegisterFaceForm";
            StartPosition = FormStartPosition.CenterScreen;
            Text = "Face Unlock Manager - Register Face";
            ResumeLayout(false);
            PerformLayout();
        }

        #endregion

        private Label LableTitle;
        private Label LableInstruct;
        private Panel PanelCamera;
        private Button BtnCancel;
        private System.Windows.Forms.Timer EmbedTimer;
        private System.Windows.Forms.Timer MonitorTimer;
        private Button BtnRegister;
        private Label label1;
    }
}