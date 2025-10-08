using Sharpmake;

[Sharpmake.Generate]
public class Zmalloc : Project
{
    public Zmalloc()
    {
        RootPath = @"[project.SharpmakeCsPath]\..\..\[project.Name]";
        SourceRootPath = @"[project.RootPath]\src";
        AddTargets(new Target(
                  // we want a target that builds for both 32 and 64-bit Windows.
                  Platform.win64,

                  // we only care about Visual Studio 2015. (Edit as needed.)
                  DevEnv.vs2022,

                  // of course, we want a debug and a release configuration.
                  Optimization.Debug | Optimization.Release));
    }

    [Configure()]
    public virtual void ConfigureAll(Configuration conf, Target target)
    {
        conf.ProjectPath = @"[project.RootPath]";
        conf.IncludePaths.Add(@"[project.SourceRootPath]");

        if (target.Optimization == Optimization.Debug)
            conf.Options.Add(Options.Vc.Compiler.RuntimeLibrary.MultiThreadedDebugDLL);
        else
            conf.Options.Add(Options.Vc.Compiler.RuntimeLibrary.MultiThreadedDLL);

        conf.Options.Add(Options.Vc.Compiler.CppLanguageStandard.Latest);


        if (target.Platform != Platform.win64)
        {
            // Exclude Windows-specific source files
            conf.SourceFilesBuildExclude.Add(@"[project.SourceRootPath]/platform_windows.cpp");
        }
        else
        {
            // Exclude non-Windows files
            conf.SourceFilesBuildExclude.Add(@"[project.SourceRootPath]/platform_linux.cpp");
        }

        conf.Output = Configuration.OutputType.Lib;
    }
}