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
        private void CopyPos()
        {
            PromptEntityOptions opts = new PromptEntityOptions("Select source object: ");
            opts.AllowNone = false;
            PromptEntityResult result = Autodesk.AutoCAD.ApplicationServices.Application.DocumentManager.MdiActiveDocument.Editor.GetEntity(opts);
            if (result.Status != PromptStatus.OK) return;

            PromptEntityOptions destopts = new PromptEntityOptions("Select destination object: ");
            destopts.AllowNone = false;
            PromptEntityResult destresult = Autodesk.AutoCAD.ApplicationServices.Application.DocumentManager.MdiActiveDocument.Editor.GetEntity(destopts);
            if (destresult.Status != PromptStatus.OK) return;
            if (result.ObjectId == destresult.ObjectId) return;

            Database db = HostApplicationServices.WorkingDatabase;
            using (Transaction tr = db.TransactionManager.StartTransaction())
            {
                try
                {
                    RebarPos source = tr.GetObject(result.ObjectId, OpenMode.ForRead) as RebarPos;
                    if (source == null) return;
                    RebarPos dest = tr.GetObject(destresult.ObjectId, OpenMode.ForWrite) as RebarPos;
                    if (dest == null) return;

                    dest.Pos = source.Pos;
                    dest.Count = source.Count;
                    dest.Diameter = source.Diameter;
                    dest.Spacing = source.Spacing;
                    dest.GroupId = source.GroupId;
                    dest.ShapeId = source.ShapeId;
                    dest.A = source.A;
                    dest.B = source.B;
                    dest.C = source.C;
                    dest.D = source.D;
                    dest.E = source.E;
                    dest.F = source.F;
                    dest.Display = source.Display;
                    dest.Note = source.Note;
                    dest.Multiplier = source.Multiplier;

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