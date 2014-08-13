using System;
using System.Collections.Generic;
using System.IO.Pipes;
using System.Linq;
using System.Security.Principal;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace SecurityCamp
{
    class Program
    {
        static void Main(string[] args)
        {
            Console.WriteLine("hello client");

            using (NamedPipeClientStream pipeClient =
                new NamedPipeClientStream(".", "testpipe",
                    PipeDirection.InOut, PipeOptions.None,
                    TokenImpersonationLevel.Impersonation))
            {

                Thread.Sleep(50000);

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
