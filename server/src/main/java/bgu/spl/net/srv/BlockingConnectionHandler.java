package bgu.spl.net.srv;


import bgu.spl.net.api.MessageEncoderDecoder;
import bgu.spl.net.api.MessagingProtocol;
import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.IOException;
import java.net.Socket;

public class BlockingConnectionHandler<T> implements Runnable, ConnectionHandler<T> {

    private final MessagingProtocol<T> protocol;
    private final MessageEncoderDecoder<T> encdec;
    private final Socket sock;
    private BufferedInputStream in;
    private BufferedOutputStream out;
    private volatile boolean connected = true;
    private final Connections<T> connections;
    private final int connectionId;

    public BlockingConnectionHandler(Socket sock, MessageEncoderDecoder<T> reader, 
                                     MessagingProtocol<T> protocol, 
                                     Connections<T> connections, int connectionId) {
        this.sock = sock;
        this.encdec = reader;
        this.protocol = protocol;
        this.connections = connections;
        this.connectionId = connectionId;
        protocol.start(connectionId, connections);
    }

    @Override
    public void run() {
        try (Socket sock = this.sock) { // Automatic closing
            in = new BufferedInputStream(sock.getInputStream());
            out = new BufferedOutputStream(sock.getOutputStream());

            while (!protocol.shouldTerminate() && connected) {
                int read = in.read();
                if (read < 0) break; // Client disconnected

                T nextMessage = encdec.decodeNextByte((byte) read);
                if (nextMessage != null) {
                    T result = protocol.process(nextMessage); // Protocol sends responses via `Connections<T>`
                    if (result != null) {
                        System.out.println(result); //printing the StompFrame
                        send(result);
                    }
                }
            }
        } catch (IOException ex) {
            ex.printStackTrace();
        } 
        // finally {
        //     closeConnection();
        // }
    }

    @Override
    public void send(T msg) {
        if (msg != null) {
            try {
                synchronized (out) { // Ensure thread safety
                    out.write(encdec.encode(msg));
                    out.flush();
                }
            } catch (IOException ex) {
                ex.printStackTrace();
                connected = false;
            }
        }
    }

    @Override
    public void close() throws IOException {
        connected = false;
        sock.close();
    }

    private void closeConnection() {
        try {
            close();
        } catch (IOException ex) {
            ex.printStackTrace();
        } finally {
            connections.disconnect(connectionId); // Remove connection from Connections
        }
    }
}