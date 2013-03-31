using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace EditorWinForms
{
    public enum Channel
    {
        MessageChannel,
        MainChannel
    }

    class EngineConnection
    {
        RakNet.RakPeerInterface peer;

        public EngineConnection(string host, ushort port)
        {
            peer = RakNet.RakPeerInterface.GetInstance();
            peer.Connect(host, port, string.Empty, 0);
        }

        public void PushMessage(Thrift.Protocol.TBase message)
        {
            //Thrift.Transport.TStreamTransport transport = new Thrift.Transport.TStreamTransport(
            //Thrift.Protocol.TBinaryProtocol protocol = new Thrift.Protocol.TBinaryProtocol(transport);
        }

        public void Send(byte[] data, int length)
        {
            peer.Send(data, length, RakNet.PacketPriority.MEDIUM_PRIORITY, RakNet.PacketReliability.RELIABLE_ORDERED, (char)Channel.MainChannel, null, true);
        }
    }

    class EngineTransport : Thrift.Transport.TTransport
    {
        RakNet.RakPeerInterface peer;

        EngineTransport(RakNet.RakPeerInterface peer)
        {
            this.peer = peer;
        }

        public override void Close()
        {
            this.peer = null;
        }

        protected override void Dispose(bool disposing)
        {
            Close();
        }

        public override bool IsOpen
        {
            get { return this.peer != null && peer.IsActive(); }
        }

        public override void Open()
        {
            peer.Send(new byte[] { (byte)1 }, 1, RakNet.PacketPriority.MEDIUM_PRIORITY, RakNet.PacketReliability.RELIABLE, (char)0, null, true);
        }

        struct Paketto
        {
            RakNet.Packet packet;
            int readOffset;

            public RakNet.Packet Packet { get { return packet; } set { packet = value; } }

            public int Read(byte[] buf, int off, int len)
            {
                int amountRead = (int)Math.Min(len, packet.length - readOffset);
                System.Buffer.BlockCopy(packet.data, 0, buf, off, amountRead);
                readOffset += amountRead;
                return amountRead;
            }

            public bool IsDone { get { return packet == null || readOffset >= packet.length; } }
        }

        Paketto currentPacket;

        public override int Read(byte[] buf, int off, int len)
        {
            if (currentPacket.IsDone)
            {
                currentPacket.Packet = peer.Receive();
                if (currentPacket.IsDone) // no more packets
                    return 0;
            }
            return currentPacket.Read(buf, off, len);
        }

        public override void Write(byte[] buf, int off, int len)
        {
            if (off == 0)
            {
                peer.Send(buf, len, RakNet.PacketPriority.MEDIUM_PRIORITY, RakNet.PacketReliability.RELIABLE, (char)Channel.MessageChannel, null, true);
            }
            else
            {
                byte[] partialBuf = new byte[len];
                System.Buffer.BlockCopy(buf, off, partialBuf, 0, len);
                peer.Send(partialBuf, len, RakNet.PacketPriority.MEDIUM_PRIORITY, RakNet.PacketReliability.RELIABLE, (char)Channel.MessageChannel, null, true);
            }
        }
    }
}
