package bgu.spl.net.impl.stomp;

import java.util.function.Supplier;

import bgu.spl.net.api.MessageEncoderDecoder;
import bgu.spl.net.api.MessagingProtocol;
import bgu.spl.net.srv.Server;

public class StompServer {

    public static void main(String[] args) {
        if (args.length < 2) {
            System.out.println("Illegal number of arguments - must be 2 (port, server type)");
            return;
        }
        int port;
        String serverType = args[1].toLowerCase();

        try {
            port = Integer.parseInt(args[0]);
        } catch (NumberFormatException e) {
            System.out.println("Invalid port number: " + args[0]);
            return;
        }

        // Create factories for protocol and encoder/decoder
        Supplier<MessagingProtocol<StompFrame>> protocolFactory = StompProtocol::new;
        Supplier<MessageEncoderDecoder<StompFrame>> encoderDecoderFactory = StompEncoderDecoder::new;

        // Start the server based on the mode
        if (serverType.equals("tpc")) {
            Server.threadPerClient(
                    port,
                    protocolFactory,
                    encoderDecoderFactory).serve();
        } else if (serverType.equals("reactor")) {
            Server.reactor(
                    Runtime.getRuntime().availableProcessors(),
                    port,
                    protocolFactory,
                    encoderDecoderFactory).serve();
        } else {
            System.out.println("Invalid server type: " + serverType + " - must be 'tpc' or 'reactor'");
        }
    }
}
