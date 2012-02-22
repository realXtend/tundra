using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Diagnostics;
using System.Threading;

namespace ConsoleApplication1
{
    class Program
    {
        static void Main(string[] args)
        {
            Console.WriteLine("ProcessStopWatch continuously measures the total uptime of a process");
            Console.WriteLine("with the given name. When the process quits, the total runtime is");
            Console.WriteLine("printed out to the console.");
            Console.WriteLine("");
            Console.WriteLine("To quit, Ctrl-C or close the window.");
            Console.WriteLine("");
            if (args.Length == 0)
            {
                Console.WriteLine("Usage: ProcessStopWatch <processname>");
                Console.WriteLine("E.g. ProcessStopWatch TundraConsole (note, omit the suffix)");
                return;
            }
            bool isRunning = false;
            long startTime = 0;

            string targetName = args[0];
            Console.WriteLine("Measuring uptimes of process '" + targetName + "'.");
            for (; ; )
            {
                Process[] processes = Process.GetProcessesByName(targetName);
                if (processes.Length > 0 && !isRunning)
                {
                    Console.WriteLine("Process " + targetName + " started.");
                    isRunning = true;
                    startTime = Stopwatch.GetTimestamp();
                }
                else if (processes.Length == 0 && isRunning)
                {
                    long stopTime = Stopwatch.GetTimestamp();
                    double seconds = (double)(stopTime - startTime) / Stopwatch.Frequency;
                    Console.WriteLine("Process " + targetName + " stopped. Uptime: " + seconds + " seconds.");
                    isRunning = false;
                }
                Thread.Sleep(1);
            }
        }
    }
}
