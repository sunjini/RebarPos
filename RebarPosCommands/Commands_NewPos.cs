﻿using System.Windows.Forms;
using Autodesk.AutoCAD.ApplicationServices;
using Autodesk.AutoCAD.DatabaseServices;
using Autodesk.AutoCAD.Geometry;
using Autodesk.AutoCAD.EditorInput;
using OZOZ.RebarPosWrapper;

namespace RebarPosCommands
{
    public partial class MyCommands
    {
        private void NewPos()
        {
            PromptPointResult result = Autodesk.AutoCAD.ApplicationServices.Application.DocumentManager.MdiActiveDocument.Editor.GetPoint("Baz noktası: ");
            if (result.Status == PromptStatus.OK)
            {
                Database db = HostApplicationServices.WorkingDatabase;
                using (Transaction tr = db.TransactionManager.StartTransaction())
                {
                    try
                    {
                        BlockTableRecord btr = (BlockTableRecord)tr.GetObject(db.CurrentSpaceId, OpenMode.ForWrite);

                        Point3d pt = result.Value;
                        RebarPos pos = new RebarPos();
                        pos.TransformBy(Matrix3d.Displacement(pt.GetAsVector()));
                        pos.TransformBy(Matrix3d.Scaling(25.0, pt));

                        pos.Pos = "X";
                        pos.Count = "XX";
                        pos.Diameter = "XX";
                        pos.Spacing = "XXX";
                        pos.Shape = "GENEL";
                        pos.A = "XXXX";
                        pos.Note = "XXXXXXXX";

                        pos.Pos = "1";
                        pos.Count = "2x3";
                        pos.Diameter = "12";
                        pos.Spacing = "200";
                        pos.Shape = "00";
                        pos.A = "1000";
                        pos.Note = "";

                        pos.SetDatabaseDefaults(db);

                        btr.AppendEntity(pos);
                        tr.AddNewlyCreatedDBObject(pos, true);

                        tr.Commit();
                    }
                    catch (System.Exception ex)
                    {
                        MessageBox.Show("Error: " + ex.Message, "RebarPos", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    }
                }
            }
        }
    }
}
