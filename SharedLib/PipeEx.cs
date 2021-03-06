﻿using System;
using System.Collections.Generic;
using System.IO;
using System.IO.Pipes;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SecurityCamp
{
    // Defines the data protocol for reading and writing strings on our stream
    //TODO 非同期対応するべきだと思います（正論
    public static class StreamStringEx
    {
        public static async Task WaitForConnectionAsync(this NamedPipeServerStream pipeServer)
        {
            await Task.Run(() => pipeServer.WaitForConnection()).ConfigureAwait(false);
        }

        static UnicodeEncoding streamEncoding = new UnicodeEncoding();
        public static string ReadString(this Stream ioStream)
        {
            int len = 0;

            len = ioStream.ReadByte() * 256;
            len += ioStream.ReadByte();
            byte[] inBuffer = new byte[len];
            ioStream.Read(inBuffer, 0, len);
            return streamEncoding.GetString(inBuffer);
        }

        public static async Task<string> ReadStringAsync(this Stream ioStream)
        {
            return await Task.Run(() => ReadString(ioStream)).ConfigureAwait(false);
        }

        public static int WriteString(this Stream ioStream, string outString)
        {
            byte[] outBuffer = streamEncoding.GetBytes(outString);
            int len = outBuffer.Length;
            if (len > UInt16.MaxValue)
            {
                len = (int)UInt16.MaxValue;
            }
            ioStream.WriteByte((byte)(len / 256));
            ioStream.WriteByte((byte)(len & 255));
            ioStream.Write(outBuffer, 0, len);
            ioStream.Flush();
            return outBuffer.Length + 2;
        }

        public static async Task<int> WriteStringAsync(this Stream ioStream, string outString)
        {
            return await Task.Run(() => WriteString(ioStream, outString)).ConfigureAwait(false);
        }
    }

    public static class TaskTimeOut
    {
        public static async Task Timeout(this Task task, TimeSpan timeout)
        {
            var delay = Task.Delay(timeout);
            if (await Task.WhenAny(task, delay) == delay)
            {
                throw new TimeoutException();
            }
        }

        public static async Task<T> Timeout<T>(this Task<T> task, TimeSpan timeout)
        {
            await ((Task)task).Timeout(timeout);
            return await task;
        }
    }
}
