using System;
using System.Drawing;
using System.Runtime.InteropServices;
using System.Windows.Forms;

namespace FaceUnlockManager
{
    //Generated using gemini pro
    internal class UIHelper
    {
        [DllImport("gdi32.dll", EntryPoint = "CreateRoundRectRgn")]
        private static extern IntPtr CreateRoundRectRgn(
            int nLeftRect,
            int nTopRect,
            int nRightRect,
            int nBottomRect,
            int nWidthEllipse,
            int nHeightEllipse
        );

        /// <summary>
        /// Dynamically applies rounded corners to any WinForms control.
        /// </summary>
        /// <param name="control">The UI control to round (e.g., Panel, Form, Button).</param>
        /// <param name="radius">The corner radius.</param>
        public static void SetRoundedCorners(Control control, int radius)
        {
            if (control == null) return;

            // Apply the rounded region immediately
            ApplyRoundedRegion(control, radius);

            // Hook into the Resize event so the rounded region updates dynamically if the control is resized
            control.Resize += (sender, e) =>
            {
                ApplyRoundedRegion(control, radius);
            };
        }

        private static void ApplyRoundedRegion(Control control, int radius)
        {
            // Dispose of the previous region object to prevent GDI memory leaks
            if (control.Region != null)
            {
                control.Region.Dispose();
            }

            // Create a new rounded region and wrap it in the C# Region object
            control.Region = Region.FromHrgn(
                CreateRoundRectRgn(0, 0, control.Width, control.Height, radius, radius)
            );
        }
    }
}
