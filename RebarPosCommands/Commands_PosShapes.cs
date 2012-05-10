﻿using System.Windows.Forms;
using Autodesk.AutoCAD.DatabaseServices;
using Autodesk.AutoCAD.Geometry;
using OZOZ.RebarPosWrapper;
using System.Collections.Generic;

namespace RebarPosCommands
{
    public partial class MyCommands
    {
        private void PosShapes()
        {
            PosShapesForm form = new PosShapesForm();

            if (form.Init())
            {
                Autodesk.AutoCAD.ApplicationServices.Application.ShowModalDialog(null, form, false);
            }
        }
    }
}