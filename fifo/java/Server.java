import java.io.FileInputStream;
import java.io.FileOutputStream;

class Server
{  
    public static void main(String args[]) throws Exception
    {
        FileInputStream server = new FileInputStream("server.fifo");
        FileOutputStream client = new FileOutputStream("client.fifo");
        byte[] data = new byte[128];

        while (true)
        {
            int len = server.read(data, 0, data.length);

            if (len < 1)
            {
                break;
            }
            client.write(data, 0, len);
        }
        server.close();
        client.close();
    }
}

