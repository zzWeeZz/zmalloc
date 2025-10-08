using System;
using Sharpmake;


[module: Sharpmake.Include(@"..\Projects\*")]
[module: Sharpmake.Include(@"..\Solutions\*")]

namespace Template
{
    public class Startup
    {
        [Sharpmake.Main]
        public static void Main(Sharpmake.Arguments arguments)
        {
            arguments.Generate<EngineSolution>();
        }
    }
}
