using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Microsoft.Win32;

namespace FaceUnlockManager {
    internal class InstallationManager {
        private const string CredentialProviderGuid = "{B84B5619-5995-4A1E-9302-F580D593DFA6}";
        private const string faceEmbeddingPath = @"C:\ProgramData\FaceUnlock\";
        private const string mainEmbeddingFile = "embedding_main.dat";
        private const string altEmbeddingFile = "embedding_alt.dat";

        public static bool IsRegistered() {

            using (RegistryKey? key = Registry.LocalMachine.OpenSubKey(
                @"SOFTWARE\Microsoft\Windows\CurrentVersion\Authentication\Credential Providers\" + CredentialProviderGuid)) {
                return key != null;
            }
        }
        public static bool IsFaceRegistered(string type = "main") {
            if(type == "main") {
                return File.Exists(faceEmbeddingPath + mainEmbeddingFile);
            }
            else if(type == "alt") {
                return File.Exists(faceEmbeddingPath + altEmbeddingFile);
            }
            return false;
        }


        public static bool DeleteData(string dataPath) {
            try {
                if (File.Exists(dataPath)) {
                    File.SetAttributes(dataPath, FileAttributes.Normal);
                    File.Delete(dataPath);
                }
                return true;
            }
            catch (Exception ex) {
                MessageBox.Show(
                    $"Error while deleting file at:\n\"{dataPath}\"\n\nDetails: {ex.Message}",
                    "File Deletion Error",
                    MessageBoxButtons.OK,
                    MessageBoxIcon.Error);
                return false;
            }
        }

        public static bool Register() {
            try {
                using (RegistryKey providerKey =
                    Registry.LocalMachine.CreateSubKey(
                        @"SOFTWARE\Microsoft\Windows\CurrentVersion\Authentication\Credential Providers\" +
                        CredentialProviderGuid)) {
                    providerKey.SetValue("", "FaceUnlockProvider");
                }

                using (RegistryKey clsidKey =
                    Registry.ClassesRoot.CreateSubKey(
                        @"CLSID\" + CredentialProviderGuid)) {
                    clsidKey.SetValue("", "FaceUnlockProvider");

                    using (RegistryKey inprocKey =
                        clsidKey.CreateSubKey("InprocServer32")) {
                        inprocKey.SetValue("", "FaceUnlock.dll");
                        inprocKey.SetValue("ThreadingModel", "Apartment");
                    }
                }

                SetDefaultCredentialProvider();

                return true;
            }
            catch (Exception ex) {
                MessageBox.Show(
                    ex.Message,
                    "Credential Provider Installation Failed");

                return false;
            }
        }

        public static bool Unregister(bool deleteSaveData = false) {
            try {
                if (InstallationManager.IsRegistered()) {
                    Registry.LocalMachine.DeleteSubKeyTree(
                        @"SOFTWARE\Microsoft\Windows\CurrentVersion\Authentication\Credential Providers\" +
                        CredentialProviderGuid,
                        false);

                    Registry.ClassesRoot.DeleteSubKeyTree(
                        @"CLSID\" + CredentialProviderGuid,
                        false);
                }

                DeleteData(faceEmbeddingPath + altEmbeddingFile);
                if (deleteSaveData) {
                    DeleteData(CredentialStorage.GetCredFilePath());
                    DeleteData(faceEmbeddingPath + mainEmbeddingFile);
                }
                ClearDefaultCredentialProvider();

                return true;
            }
            catch (Exception ex) {
                MessageBox.Show(
                    ex.Message,
                    "Credential Provider Uninstallation Failed");

                return false;
            }
        }

        public static bool SetDefaultCredentialProvider() {
            try {
                using RegistryKey? key =
                    Registry.LocalMachine.CreateSubKey(
                        @"SOFTWARE\Policies\Microsoft\Windows\System");

                if (key == null)
                    return false;

                key.SetValue(
                    "DefaultCredentialProvider",
                    CredentialProviderGuid,
                    RegistryValueKind.String);

                return true;
            }
            catch (Exception ex) {
                MessageBox.Show(ex.Message);
                return false;
            }
        }

        public static void ClearDefaultCredentialProvider() {
            using RegistryKey? key =
                Registry.LocalMachine.CreateSubKey(
                    @"SOFTWARE\Policies\Microsoft\Windows\System");

            key?.DeleteValue("DefaultCredentialProvider", false);
        }
    }
}
