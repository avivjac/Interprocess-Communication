package bgu.spl.net.srv;

import bgu.spl.net.api.MessageEncoderDecoder;
import bgu.spl.net.api.MessagingProtocol;
import bgu.spl.net.impl.stomp.StompFrame;

import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.function.Supplier;

public class BaseServer implements Server<StompFrame> {
    // fields
    private final int port;
    private final Supplier<MessagingProtocol<StompFrame>> protocolFactory;
    private final Supplier<MessageEncoderDecoder<StompFrame>> encdecFactory;
    private ServerSocket sock;
    private Connections<StompFrame> connections;
    private AtomicInteger connectionIdCounter;

    // constructor
    public BaseServer(int port, Supplier<MessagingProtocol<StompFrame>> protocolFactory,
            Supplier<MessageEncoderDecoder<StompFrame>> encdecFactory) {

        this.port = port;
        this.protocolFactory = protocolFactory;
        this.encdecFactory = encdecFactory;
        this.sock = null;
        this.connections = new ConnectionsImpl<>();
        this.connectionIdCounter = new AtomicInteger(0);
    }

    @Override
    public void serve() {

        try (ServerSocket serverSock = new ServerSocket(port)) {
            System.out.println("Server started on port " + port);

            this.sock = serverSock; // Just to be able to close it later

            while (!Thread.currentThread().isInterrupted()) {
                Socket clientSock = serverSock.accept();
                int connectionId = connectionIdCounter.getAndIncrement();
                BlockingConnectionHandler<StompFrame> handler = new BlockingConnectionHandler<>(clientSock,
                        encdecFactory.get(), protocolFactory.get(), connections, connectionId);

                connections.addConnection(connectionId, handler);

                execute(handler);
            }
        } catch (IOException ex) {
            ex.printStackTrace();
        }

        System.out.println("Server closed");
    }

    @Override
    public void close() throws IOException {
        if (sock != null) {
            sock.close();
        }
    }

    protected void execute(BlockingConnectionHandler<StompFrame> handler) {
        new Thread(handler).start();
    }

}
