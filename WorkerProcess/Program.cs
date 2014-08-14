using System;
using System.Collections.Generic;
using System.IO.Pipes;
using System.Linq;
using System.Runtime.InteropServices;
using System.Security.Principal;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace SecurityCamp
{
    class Program
    {
        [DllImport(@"C:\Users\tnkt\Documents\Visual Studio 2013\Projects\MasterWorkerProcessTest\Release\CalledByCSLibTest.dll", CharSet = CharSet.Ansi)]
        static extern void CoutEndl(string str);

        [DllImport(@"C:\Users\tnkt\Documents\Visual Studio 2013\Projects\MasterWorkerProcessTest\Release\CalledByCSLibTest.dll")]
        static extern int RestTest();

        static void Main(string[] args)
        {
            Console.WriteLine("hello client");
            Console.WriteLine("print dll message");

            CoutEndl("ハロー！きんいろモザイク");
            CoutEndl("Hello! KIN-MOZA");
            RestTest();

            Console.WriteLine("dll executed");
            Console.ReadLine();

            using (NamedPipeClientStream pipeClient =
                new NamedPipeClientStream(".", "testpipe",
                    PipeDirection.InOut, PipeOptions.None,
                    TokenImpersonationLevel.Impersonation))
            {

                Console.WriteLine("Connecting to server...\n");
                pipeClient.Connect();
                //StreamString ss = new StreamString(pipeClient);
                // Validate the server's signature string
                if (pipeClient.ReadString() == "I am the one true server!")
                {
                    var message = pipeClient.ReadString();
                    var response = message.Substring(1, message.Length - 1);
                    
                    //Console.WriteLine("thread sleep");
                    //Thread.Sleep(3000);
                    
                    pipeClient.WriteString(response);
                }
                else
                {
                    Console.WriteLine("Server could not be verified.");
                }
                // Give the client process some time to display results before exiting.
                //Thread.Sleep(4000);

            }
            Console.WriteLine("To close press enter...");
            Console.ReadLine();
        }
    }
}
