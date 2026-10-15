using System;
using System.Collections.Generic;
using System.DirectoryServices.AccountManagement;
using System.Linq;
using System.Security.Principal;
using System.Text;
using System.Threading.Tasks;

namespace FaceUnlockManager {
    internal class UserAuth {

        public enum AccountType {
            Local,
            MicrosoftAccount,
            Unknown
        }

        public static AccountType GetCurrentAccountType() {
            string currentSamUser = Environment.UserName;

            try {
                using (PrincipalContext context = new PrincipalContext(ContextType.Machine)) {
                    UserPrincipal user = UserPrincipal.FindByIdentity(context, currentSamUser);

                    if (user != null) {
                        if (!string.IsNullOrEmpty(user.UserPrincipalName) && user.UserPrincipalName.Contains("@")) {
                            return AccountType.MicrosoftAccount;
                        }
                        return AccountType.Local;
                    }
                }
            }
            catch (Exception ex) {
                MessageBox.Show("Unable to detect user type : \n" + ex.Message, "Error");
                return AccountType.Unknown;
            }

            return AccountType.Unknown;
        }

        public static class AccountDetector {
            public static AccountType GetCurrentAccountType() {
                try {
                    WindowsIdentity identity = WindowsIdentity.GetCurrent();

                    if (identity == null || identity.User == null)
                        return AccountType.Unknown;

                    string sid = identity.User.Value;

                    // Microsoft personal accounts use SIDs beginning with S-1-12-1
                    if (sid.StartsWith("S-1-12-1")) {
                        return AccountType.MicrosoftAccount;
                    }

                    // Normal local Windows accounts use S-1-5-21-...
                    if (sid.StartsWith("S-1-5-21-")) {
                        return AccountType.Local;
                    }

                    return AccountType.Unknown;
                }
                catch {
                    return AccountType.Unknown;
                }
            }
        }

        public static string GetCurrentUPN() {
            string currentSamUser = Environment.UserName;

            try {
                using (PrincipalContext context = new PrincipalContext(ContextType.Machine)) {
                    UserPrincipal user = UserPrincipal.FindByIdentity(context, currentSamUser);

                    if (user != null) {
                        if (!string.IsNullOrEmpty(user.UserPrincipalName) && user.UserPrincipalName.Contains("@")) {
                            return user.UserPrincipalName;
                        }
                        else {
                            return null;
                        }
                    }
                    else {
                        return null;
                    }
                }
            }
            catch (Exception ex) {
                MessageBox.Show("Unable to detect user type : \n" + ex.Message, "Error");
                return null;
            }
        }


        public static async Task<bool> VerifyUsernamePassword(string username, string password) {
            if (GetCurrentAccountType() == AccountType.MicrosoftAccount) { 
                return true;
            }
            return await Task.Run(() => {
                try {
                    using (PrincipalContext context = new PrincipalContext(ContextType.Machine)) {
                        return context.ValidateCredentials(username, password);
                    }
                }
                catch (Exception) {
                    return false;
                }
            });
        }

    }
}
