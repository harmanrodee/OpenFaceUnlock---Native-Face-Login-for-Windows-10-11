using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Microsoft.Win32;

namespace FaceUnlockManager {
    internal class RegistryHelper {
        private const string RegistryPath = @"SOFTWARE\FaceUnlock";
        private const string InstallDir = "Installdir";

        public static string GetInstallDir () {
            using (RegistryKey? key = Registry.LocalMachine.OpenSubKey(RegistryPath)) {
                if (key == null) {
                    return null;
                }else {
                    return key.GetValue(InstallDir) as string;
                }
            } 
        }
    }
}
