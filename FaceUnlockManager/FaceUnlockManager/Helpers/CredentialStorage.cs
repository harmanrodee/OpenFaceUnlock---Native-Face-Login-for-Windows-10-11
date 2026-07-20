using System;
using System.IO;
using System.Security.Cryptography;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.Json;
using System.Threading.Tasks;
using System.Reflection.Metadata.Ecma335;
using System.Drawing.Text;
using System.Security.AccessControl;
using System.Security.Principal;

namespace FaceUnlockManager {

    internal class CredentialStorage {
        private class CredentailData {
            public string username { get; set; }
            public string password { get; set; }
        }

        public static string GetCredFilePath() {
            string folder = @"C:\ProgramData\FaceUnlock";

            Directory.CreateDirectory(folder);
            SecureFolder(folder);

            return Path.Combine(folder, "credentials.dat");
        }
        private static void SecureFolder(string folderPath)
        {
            try
            {
                DirectoryInfo dInfo = new DirectoryInfo(folderPath);
                
                dInfo.Attributes |= FileAttributes.Hidden;

                DirectorySecurity dSecurity = dInfo.GetAccessControl();

                dSecurity.SetAccessRuleProtection(true, false);

                dSecurity.AddAccessRule(new FileSystemAccessRule(
                    new SecurityIdentifier(WellKnownSidType.LocalSystemSid, null),
                    FileSystemRights.FullControl,
                    InheritanceFlags.ContainerInherit | InheritanceFlags.ObjectInherit,
                    PropagationFlags.None,
                    AccessControlType.Allow));

                dSecurity.AddAccessRule(new FileSystemAccessRule(
                    new SecurityIdentifier(WellKnownSidType.BuiltinAdministratorsSid, null),
                    FileSystemRights.FullControl,
                    InheritanceFlags.ContainerInherit | InheritanceFlags.ObjectInherit,
                    PropagationFlags.None,
                    AccessControlType.Allow));

                dSecurity.AddAccessRule(new FileSystemAccessRule(
                    new SecurityIdentifier(WellKnownSidType.BuiltinUsersSid, null),
                    FileSystemRights.ReadAndExecute,
                    InheritanceFlags.ContainerInherit | InheritanceFlags.ObjectInherit,
                    PropagationFlags.None,
                    AccessControlType.Allow));

                dInfo.SetAccessControl(dSecurity);
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine("Error securing folder: " + ex.Message);
            }
        }

        public static bool isCredSaved() {
            return File.Exists(GetCredFilePath());
        }

        private static byte[] Encrypt(byte[] data) {
            return ProtectedData.Protect(data, null, DataProtectionScope.LocalMachine);
        }

        public static void SaveEncryptedCred(string username, string password) {
            CredentailData cred = new CredentailData();
            cred.username = username;
            cred.password = password;

            string json = JsonSerializer.Serialize(cred);

            byte[] plaindata = Encoding.UTF8.GetBytes(json);
            byte[] encrypteddata = Encrypt(plaindata);

            File.WriteAllBytes(GetCredFilePath(), encrypteddata);

        }
    }
}
