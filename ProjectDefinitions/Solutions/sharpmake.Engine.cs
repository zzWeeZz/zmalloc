using Sharpmake;
using System;

[Sharpmake.Generate]
public class EngineSolution : Sharpmake.Solution
{
    public String WorkingDirectory = @"[solution.SharpmakeCsPath]\..";
    public EngineSolution()
    {
        Name = "Engine";
        AddTargets(new Target(Platform.win64, DevEnv.vs2022, Optimization.Debug | Optimization.Release));
    }

    [Configure()]
    public void ConfigureAll(Configuration conf, Target target)
    {
        conf.SolutionFileName = "[solution.Name]";
        conf.SolutionPath = WorkingDirectory;
        conf.AddProject<EntryPoint>(target);
        conf.AddProject<Core>(target);
    }
}