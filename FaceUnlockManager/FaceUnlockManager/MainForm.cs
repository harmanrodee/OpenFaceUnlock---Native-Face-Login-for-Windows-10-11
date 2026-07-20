using System.IO;
using System.Diagnostics;
using System.Runtime.InteropServices;
using Microsoft.VisualBasic.Logging;

namespace FaceUnlockManager
{
    public partial class MainForm : Form {
        [DllImport("winmm.dll", EntryPoint = "timeBeginPeriod")]
        private static extern uint TimeBeginPeriod(uint uMilliseconds);

        [DllImport("winmm.dll", EntryPoint = "timeEndPeriod")]
        private static extern uint TimeEndPeriod(uint uMilliseconds);

        private const int MAX_PASSWORD_ATTEMPTS = 3;
        private int remainingPassAttempt = MAX_PASSWORD_ATTEMPTS;
        private string localUserName = Environment.UserName;
        private string version = Application.ProductVersion;

        private enum PassPanelRunOptions {
            RUN_AS_REGISTER,
            RUN_AS_PASS,
            RUN_AS_DERESGISTER,
            RUN_AS_NONE
        }

        PassPanelRunOptions passPanelRunAs = PassPanelRunOptions.RUN_AS_NONE;

        public async void ShowToast(string toastMessage, string icon = "success") {
            ToastText.Text = toastMessage;
            if (icon == "success") {
                ToastIcon.Image = Properties.Resources.tick_filled;
            }
            else if (icon == "error") {
                ToastIcon.Image = Properties.Resources.cross_filled;
            }
            else if (icon == "disable") {
                ToastIcon.Image = Properties.Resources.disable;
            }

            int targetX = 10;
            int targetY = 20;

            int offscreenX = -PanelToast.Width;
            PanelToast.Location = new Point(offscreenX, targetY);
            PanelToast.Visible = true;
            PanelToast.BringToFront();

            int slideDurationMs = 400;
            int targetFps = 120;
            int frameDelayMs = 1000 / targetFps;
            int totalFrames = slideDurationMs / frameDelayMs;

            for (int frame = 1; frame <= totalFrames; frame++) {
                float progress = (float)frame / totalFrames;

                float easeOutProgress = 1f - (1f - progress) * (1f - progress);

                int currentX = (int)(offscreenX + (targetX - offscreenX) * easeOutProgress);
                PanelToast.Location = new Point(currentX, targetY);

                await Task.Delay(frameDelayMs);
            }

            PanelToast.Location = new Point(targetX, targetY);

            await Task.Delay(2000);

            for (int frame = 1; frame <= totalFrames; frame++) {
                float progress = (float)frame / totalFrames;

                float easeInProgress = progress * progress;

                int currentX = (int)(targetX - (targetX - offscreenX) * easeInProgress);
                PanelToast.Location = new Point(currentX, targetY);

                await Task.Delay(frameDelayMs);
            }

            PanelToast.Location = new Point(offscreenX, targetY);
            PanelToast.Visible = false;
        }

        private void UpdateFormStates() {
            //update face registered status
            if (InstallationManager.IsFaceRegistered()) {
                PicBoxFaceReg.Image = Properties.Resources.tick;
                if (!InstallationManager.IsRegistered() && CredentialStorage.isCredSaved()) {
                    InstallationManager.Register();
                }
            }
            else {
                PicBoxFaceReg.Image = Properties.Resources.cross;
                InstallationManager.Unregister();
            }

            //enable/disbale Face Register Button depending upon credentail.dat file exists
            if (!CredentialStorage.isCredSaved()) {
                InstallationManager.Unregister();
            }

            //update active status
            if (InstallationManager.IsRegistered()) {
                PicBoxActive.Image = Properties.Resources.tick;
            }
            else {
                PicBoxActive.Image = Properties.Resources.cross;
            }

        }

        public MainForm() {
            TimeBeginPeriod(1);
            InitializeComponent();
            LabelVersion.Text = $"Version: {version}";
            LabelAccName.Text = $"User : {localUserName}";
            UpdateFormStates();
            
            // Clean up the FaceUnlockFirstStart scheduled task after it runs once on reboot
            Task.Run(() => {
                try {
                    Process process = new Process();
                    process.StartInfo.FileName = "schtasks";
                    process.StartInfo.Arguments = "/delete /tn \"FaceUnlockFirstStart\" /f";
                    process.StartInfo.UseShellExecute = true;
                    process.StartInfo.WindowStyle = ProcessWindowStyle.Hidden;
                    process.Start();
                } catch { }
            });

            UIHelper.SetRoundedCorners(PanelStatus, 20);
            UIHelper.SetRoundedCorners(PanelToast, 20);
            UIHelper.SetRoundedCorners(PanelPassword, 20);
        }

        private void BtnRegister_Click(object sender, EventArgs e) {
            passPanelRunAs = PassPanelRunOptions.RUN_AS_REGISTER;
            PanelPassword.Visible = true;
        }

        private async void BtnPassPnlConf_Click(object sender, EventArgs e) {
            if (remainingPassAttempt != 0) {
                remainingPassAttempt--;
                string enteredPass = "";
                enteredPass = PasswordBox.Text;
                if (!(enteredPass.Length > 0)) {
                    return;
                }
                PasswordBox.Text = "";
                BtnPassPnlConf.Enabled = false;
                bool isVerified = await UserAuth.VerifyUsernamePassword(localUserName, enteredPass);
                if (isVerified) {
                    switch (passPanelRunAs) {
                        case PassPanelRunOptions.RUN_AS_REGISTER: {
                                CredentialStorage.SaveEncryptedCred(localUserName, enteredPass);
                                RegisterFaceForm form = new RegisterFaceForm(localUserName, enteredPass, this);
                                form.FormClosed += RegisterFaceForm_FormClosed;
                                form.ShowDialog();
                                break;
                            }
                        case PassPanelRunOptions.RUN_AS_PASS: {
                                CredentialStorage.SaveEncryptedCred(localUserName, enteredPass);
                                ShowToast("Password Updated");
                                break;
                            }
                        case PassPanelRunOptions.RUN_AS_DERESGISTER: {
                                InstallationManager.Unregister(true);
                                ShowToast("Unregistered", "disable");
                                break;
                            }
                        case PassPanelRunOptions.RUN_AS_NONE: {
                                break;
                            }
                    }
                    PanelPassword.Visible = false;
                    LabelAttemptWarn.Visible = false;
                    remainingPassAttempt = MAX_PASSWORD_ATTEMPTS;
                }
                else {
                    ShowToast("Incorrect Password", "error");
                    LabelAttemptWarn.Visible = true;
                    LabelAttemptWarn.Text = $"{remainingPassAttempt} attempts remaining";
                }
                BtnPassPnlConf.Enabled = true;
                UpdateFormStates();
            }
            else {
                BtnPassword.Enabled = false;
            }
        }

        private void BtnPassPnlExt_Click(object sender, EventArgs e) {
            PanelPassword.Visible = false;
            LabelAttemptWarn.Visible = false;
        }

        private void BtnPassword_Click(object sender, EventArgs e) {
            passPanelRunAs = PassPanelRunOptions.RUN_AS_PASS;
            PanelPassword.Visible = true;
        }

        private void RegisterFaceForm_FormClosed(object sender, FormClosedEventArgs e) {
            UpdateFormStates();
        }

        private void BtnRemove_Click(object sender, EventArgs e) {
            passPanelRunAs = PassPanelRunOptions.RUN_AS_DERESGISTER;
            PanelPassword.Visible = true;
            UpdateFormStates();
        }

        private void PanelPassword_VisibleChanged(object sender, EventArgs e) {
            if (PanelPassword.Visible) {
                PanelMainUI.Visible = false;
                PasswordBox.Focus();
                this.BackColor = Color.Gray;
            }
            else {
                PasswordBox.Text = "";
                PanelMainUI.Visible = true;
                this.BackColor = Color.WhiteSmoke;
            }
        }

        private void BtnPassPnlConf_EnabledChanged(object sender, EventArgs e) {
            if (BtnPassPnlConf.Enabled) {
                BtnPassPnlConf.ForeColor = Color.WhiteSmoke;
                BtnPassPnlConf.Text = "Confirm";
            }
            else {
                BtnPassPnlConf.ForeColor = Color.Gray;
                BtnPassPnlConf.Text = "Verifying";
            }
        }

        private void PasswordBox_KeyDown(object sender, KeyEventArgs e) {
            if(e.KeyCode == Keys.Enter) {
                BtnPassPnlConf_Click(sender, e);
            }
        }
    }
}
