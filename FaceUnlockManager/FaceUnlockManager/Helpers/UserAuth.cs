using System;
using System.DirectoryServices.AccountManagement;
using System.Collections.Generic;
using System.Linq;
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
