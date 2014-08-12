using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.IO.Pipes;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace SecurityCamp
{
    /// <summary>
    /// MainWindow.xaml の相互作用ロジック
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
        }

        private void readFileButton_Click(object sender, RoutedEventArgs e)
        {
            MessageBox.Show(File.ReadAllText("testfile.txt"));
        }

        private async void runWorkerProcessButton_Click(object sender, RoutedEventArgs e)
        {
            var path = @"..\..\..\WorkerProcess\bin\Debug\WorkerProcess.exe";
            var child = Process.Start(path, "spawnclient");
            
            await CommunicateAsync();
            
            if(!child.HasExited) child.CloseMainWindow();
            child.Dispose();
        }

        private async Task CommunicateAsync()
        {
            using (NamedPipeServerStream pipeServer = new NamedPipeServerStream("testpipe", PipeDirection.InOut))
            {
                // Wait for a client to connect
                pipeServer.WaitForConnection();
                try
                {
                    // Read the request from the client. Once the client has
                    // written to the pipe its security token will be available.

                    //StreamString ss = new StreamString(pipeServer);
                    await pipeServer.WriteStringAsync("I am the one true server!");//これいる？コマンドライン引数に適当な数値とか入れて名前変えたほうが良さげ

                    await pipeServer.WriteStringAsync(textBox.Text.Length == 0 ? "hoge" : textBox.Text);
                    var message = await pipeServer.ReadStringAsync();
                    //var message = pipeServer.ReadString();
                    MessageBox.Show(message);
                }
                // Catch the IOException that is raised if the pipe is broken
                // or disconnected.
                catch (IOException e)
                {
                    Console.WriteLine("ERROR: {0}", e.Message);
                }
            }
        }
    }
}
