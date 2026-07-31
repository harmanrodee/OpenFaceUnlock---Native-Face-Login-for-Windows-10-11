using System;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Runtime.InteropServices;
using System.Windows.Forms;

namespace FaceUnlockManager {
    public partial class RegisterFaceForm : Form {

        [DllImport("user32.dll", SetLastError = true)]
        static extern IntPtr FindWindow(string lpClassName, string lpWindowName);

        [DllImport("user32.dll", SetLastError = true)]
        static extern IntPtr SetParent(IntPtr hWndChild, IntPtr hWndNewParent);

        [DllImport("user32.dll", SetLastError = true)]
        static extern bool MoveWindow(IntPtr hWnd, int X, int Y, int nWidth, int nHeight, bool Repaint);

        [DllImport("user32.dll", SetLastError = true)]
        static extern int GetWindowLong(IntPtr hWnd, int nIndex);

        [DllImport("user32.dll", SetLastError = true)]
        static extern int SetWindowLong(IntPtr hWnd, int nIndex, int dwNewLong);

        [DllImport("user32.dll", SetLastError = true)]
        static extern bool IsWindow(IntPtr hWnd);

        const int GWL_STYLE = -16;
        const int WS_CAPTION = 0x00C00000;
        const int WS_THICKFRAME = 0x00040000;
        const int WS_MINIMIZEBOX = 0x00020000;
        const int WS_MAXIMIZEBOX = 0x00010000;
        const int WS_SYSMENU = 0x00080000;

        private string username = "";
        private string password = "";
        private UserAuth.AccountType userAccountType = UserAuth.AccountType.Unknown;

        private Process faceUnlockProcess = null;
        private IntPtr embeddedWindowHandle = IntPtr.Zero;
        private MainForm mainForm = null;
        private string savetype;

        public RegisterFaceForm(string username, string password, MainForm form, string savetype) {
            InitializeComponent();
            this.mainForm = form;
            mainForm.Visible = false;
            string upn = UserAuth.GetCurrentUPN();
            this.username = string.IsNullOrEmpty(upn) ? username : upn;
            this.password = password;
            this.savetype = savetype;
            userAccountType = UserAuth.GetCurrentAccountType();
            UIHelper.SetRoundedCorners(PanelCamera, 10);
            UIHelper.SetRoundedCorners(BtnRegister, 10);
            UIHelper.SetRoundedCorners(BtnCancel, 10);
            BtnRegister.Focus();
        }

        private void EmbedTimer_Tick(object sender, EventArgs e) {
            IntPtr faceWindow = FindWindow(null, "FaceUnlockRegister");

            if (faceWindow != IntPtr.Zero) {
                EmbedTimer.Stop();
                embeddedWindowHandle = faceWindow;

                SetParent(faceWindow, PanelCamera.Handle);

                int style = GetWindowLong(faceWindow, GWL_STYLE);
                style &= ~WS_CAPTION;
                style &= ~WS_THICKFRAME;
                style &= ~WS_MINIMIZEBOX;
                style &= ~WS_MAXIMIZEBOX;
                style &= ~WS_SYSMENU;

                SetWindowLong(faceWindow, GWL_STYLE, style);

                MoveWindow(
                    faceWindow,
                    0,
                    0,
                    PanelCamera.Width,
                    PanelCamera.Height,
                    true);

                MonitorTimer.Start();
            }
        }

        private void MonitorTimer_Tick(object sender, EventArgs e) {
            if (embeddedWindowHandle != IntPtr.Zero && !IsWindow(embeddedWindowHandle)) {
                MonitorTimer.Stop();
                CloseFormSafely();
            }
        }

        private void FaceUnlockProcess_Exited(object sender, EventArgs e) {
            switch (faceUnlockProcess.ExitCode) {
                case 100: {
                        CloseFormSafely();
                        mainForm.ShowToast("Face Registered", "success");
                        break;
                    }
                case 0: {
                        CloseFormSafely();
                        mainForm.ShowToast("No Stable Face Detected", "error");
                        break;
                    }
                default: {
                        CloseFormSafely();
                        break;
                    }
            }
        }

        private void CloseFormSafely() {
            MonitorTimer.Stop();
            EmbedTimer.Stop();
            this.Close();
            mainForm.Visible = true;
        }

        private void StartFaceRegister() {
            string installDir = RegistryHelper.GetInstallDir();
            if (installDir == null) {
                MessageBox.Show("Face Unlock not installed!\nReinstall the program to fix the error", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                this.Close();
            }
            else {
                string exePath = Path.Combine(installDir, "FaceUnlock.exe");
                if (!File.Exists(exePath)) {
                    MessageBox.Show("FaceUnlock.exe not found!\nReinstall the program to fix the error", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    this.Close();
                }
                else {
                    ProcessStartInfo psi = new ProcessStartInfo();
                    psi.FileName = exePath;
                    psi.Arguments = $"--mode register --user \"{username}\" --pass \"{password}\" --type \"{savetype}\"";
                    psi.WorkingDirectory = installDir;
                    psi.UseShellExecute = false;
                    psi.CreateNoWindow = true;
                    psi.WindowStyle = ProcessWindowStyle.Hidden;
                    psi.RedirectStandardInput = true;

                    // Store process reference and handle exit event
                    faceUnlockProcess = new Process();
                    faceUnlockProcess.StartInfo = psi;
                    faceUnlockProcess.EnableRaisingEvents = true;
                    faceUnlockProcess.SynchronizingObject = this;
                    faceUnlockProcess.Exited += FaceUnlockProcess_Exited;

                    faceUnlockProcess.Start();
                    EmbedTimer.Start();
                }
            }
        }

        private void BtnRegister_Click(object sender, EventArgs e) {
            BtnRegister.Enabled = false;
            if (userAccountType != UserAuth.AccountType.Unknown) {
                StartFaceRegister();
            }
            else {
                MessageBox.Show("Unknown Account Type", "Error");
                this.Close();
            }
        }

        private void BtnCancel_Click(object sender, EventArgs e) {
            if (faceUnlockProcess != null && !faceUnlockProcess.HasExited) {
                try {
                    faceUnlockProcess.StandardInput.WriteLine("exit");
                }
                catch (Exception) { }
            }
            CloseFormSafely();
        }

    }
}