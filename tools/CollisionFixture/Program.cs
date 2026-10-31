using System;
using System.IO;
using Effekseer;
using Effekseer.Data;

// Original, texture-free rain fixture authored for this bridge.
Core.New();
var node=(Node)Core.Root.Children[0];
node.CommonValues.MaxGeneration.Value.SetValue(400);
node.CommonValues.Life.SetCenter(240);
node.CommonValues.GenerationTime.SetCenter(0.5f);
node.LocationValues.Type.SetValue(LocationValues.ParamaterType.PVA);
node.LocationValues.PVA.Location.X.SetMax(5.0f);
node.LocationValues.PVA.Location.X.SetMin(-5.0f);
node.LocationValues.PVA.Location.Y.SetCenter(5.0f);
node.LocationValues.PVA.Location.Z.SetMax(1.0f);
node.LocationValues.PVA.Location.Z.SetMin(-1.0f);
node.LocationValues.PVA.Velocity.Y.SetCenter(-0.08f);
node.LocationValues.PVA.Acceleration.Y.SetCenter(-0.004f);
node.ScalingValues.Fixed.Scale.X.SetValue(0.12f);
node.ScalingValues.Fixed.Scale.Y.SetValue(0.12f);
node.ScalingValues.Fixed.Scale.Z.SetValue(0.12f);
var output=Path.GetFullPath(args[0]);
File.WriteAllBytes(output,new Effekseer.Binary.Exporter().Export(Core.Root,1.0f));
Console.WriteLine(output);
