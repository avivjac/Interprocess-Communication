package bgu.spl.net.impl.stomp;

import bgu.spl.net.api.MessagingProtocol;
import bgu.spl.net.srv.ConnectionsImpl;
import bgu.spl.net.srv.Connections;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

public class StompProtocol implements MessagingProtocol<StompFrame> {

    private boolean shouldTerminate = false;
    private int connectionId;
    private Connections<StompFrame> connections;
    private String username;
    private Map<String, String> subscriptions; // Subscription ID to channel mapping

    public StompProtocol() {
        subscriptions = new ConcurrentHashMap<>();
    }

    @Override
    public void start(int connectionId, Connections<StompFrame> connections) {
        this.connectionId = connectionId;
        this.connections = connections;
    }

    @Override
    public StompFrame process(StompFrame msg) {
        String command = msg.getCommand();

        switch (command) {
            case "CONNECT":
                return handleConnect(msg);
            case "SUBSCRIBE":
                return handleSubscribe(msg);
            case "UNSUBSCRIBE":
                return handleUnsubscribe(msg);
            case "SEND":
                return handleSend(msg);
            case "DISCONNECT":
                return handleDisconnect(msg);
            default:
                return createErrorFrame("Unknown command: " + command);
        }
    }

    @Override
    public boolean shouldTerminate() {
        return shouldTerminate;
    }

    private StompFrame handleConnect(StompFrame msg) {
        //Case 2 - The client is already logged in
        System.out.println("somenone is trying to connect");
        if (username != null) {
            return createErrorFrame("The client is already logged in, log out before trying again");
        }

        String login = msg.getHeader("login");
        String passcode = msg.getHeader("passcode");

        //case 1 - farme validation 
        if (login == null || passcode == null) {
            
            return createErrorFrame("Could not connect to server");
        }

        ConnectionsImpl<StompFrame> connectionsImpl = (ConnectionsImpl<StompFrame>) connections;

        //case 5
        if (!connectionsImpl.registerUser(login, passcode) && !connectionsImpl.authenticateUser(login, passcode)) {
            return createErrorFrame("Wrong password");
        }

        //Case 4
        if (!connectionsImpl.loginUser(login, connectionId)) {
            return createErrorFrame("User already logged in.");
        }

        username = login;
        //case 6
        StompFrame ret = new StompFrame("CONNECTED");
        ret.addHeader("version", "1.2");

        return ret; //return the stomp frame
    }

    private StompFrame handleSubscribe(StompFrame msg) {
        if (username == null) {
            return createErrorFrame("User not logged in. Please CONNECT first.");
        }
        String destination = msg.getHeader("destination");
        String subscriptionId = msg.getHeader("id");

        if (destination == null || subscriptionId == null) {
            return createErrorFrame("SUBSCRIBE frame missing 'destination' or 'id' header.");
        }
        // ido added a print
        System.out.println(subscriptionId+" is subscribing to " + destination.toString());

        subscriptions.put(subscriptionId, destination);
        ((ConnectionsImpl<StompFrame>) connections).subscribeToChannel(destination, connectionId);

        return createReceiptFrame(msg.getHeader("receipt"));
    }

    private StompFrame handleUnsubscribe(StompFrame msg) {
        if (username == null) {
            return createErrorFrame("User not logged in. Please CONNECT first.");
        }
        String subscriptionId = msg.getHeader("id");

        if (subscriptionId == null || !subscriptions.containsKey(subscriptionId)) {
            return createErrorFrame("UNSUBSCRIBE frame missing 'id' or invalid subscription.");
        }
        
        String destination = subscriptions.remove(subscriptionId);
        ((ConnectionsImpl<StompFrame>) connections).unsubscribeFromChannel(destination, connectionId);

        // ido added a print
        System.out.println(subscriptionId+" is unsubscribing from " + destination.toString());

        return createReceiptFrame(msg.getHeader("receipt"));
    }

    private StompFrame handleSend(StompFrame msg) {
        if (username == null) {
            return createErrorFrame("User not logged in. Please CONNECT first.");
        }
        String destination = msg.getHeader("destination");

        if (destination == null) {
            return createErrorFrame("SEND frame missing 'destination' header.");
        }
        // ido added a print
        System.out.println("someone is sending to " + destination.toString() + "the messege: "+msg.getBody());

        //send the meassage to all the subscribers
        ((ConnectionsImpl<StompFrame>) connections).send(destination.toString() ,msg); //WHY THE FUCK THIS LINE DONT WORK

        return createReceiptFrame(msg.getHeader("receipt"));
    }

    private StompFrame handleDisconnect(StompFrame msg) {
        if (username == null) {
            return createErrorFrame("User not logged in. Please CONNECT first.");
        }
        shouldTerminate = true;

        // Remove the user from active users
        ((ConnectionsImpl<StompFrame>) connections).send(this.connectionId, createReceiptFrame(msg.getHeader("receipt")));
        ((ConnectionsImpl<StompFrame>) connections).logoutUser(username);

        // Clear all subscriptions
        subscriptions.clear();

        // ido added a print
        System.out.println("someone is dissconnecting");

        return null;
    }

    private StompFrame createReceiptFrame(String receiptId) {
        if (receiptId != null) {
            StompFrame receiptFrame = new StompFrame("RECEIPT");
            receiptFrame.addHeader("receipt-id", receiptId);
            return receiptFrame;
        }
        return null;
    }

    private StompFrame createErrorFrame(String message) {
        StompFrame errorFrame = new StompFrame("ERROR");
        errorFrame.addHeader("message", message);
        errorFrame.setBody("Error: " + message);
        return errorFrame;
    }
}