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
                return createErrorFrame("Unknown command: " + command, null, msg);
        }
    }

    @Override
    public boolean shouldTerminate() {
        return shouldTerminate;
    }

    private StompFrame handleConnect(StompFrame msg) {
        // Case 2 - The client is already logged in

        if (username != null) {
            return createErrorFrame("The client is already logged in, log out before trying again", null, msg);
        }

        String login = msg.getHeader("login");
        String passcode = msg.getHeader("passcode");

        // case 1 - farme validation
        if (login.equals(("")) || passcode.equals("") || login == null || passcode == null) {
            return createErrorFrame("Could not connect to server", null, msg);
        }
        ConnectionsImpl<StompFrame> connectionsImpl = (ConnectionsImpl<StompFrame>) connections;
        // case 5
        if (!connectionsImpl.registerUser(login, passcode) && !connectionsImpl.authenticateUser(login, passcode)) {
            return createErrorFrame("Wrong password", null, msg);
        }
        // Case 4
        if (!connectionsImpl.loginUser(login, connectionId)) {
            return createErrorFrame("User already logged in.", null, msg);
        }
        this.username = login;
        // case 6
        StompFrame ret = new StompFrame("CONNECTED");
        ret.addHeader("version", "1.2");
        System.out.println("User " + username + " connected");
        return ret; // return the stomp frame
    }

    private StompFrame handleSubscribe(StompFrame msg) {
        String receiptID = msg.getHeader("receipt");
        if (username == null) {
            return createErrorFrame("User not logged in. Please CONNECT first.", receiptID, msg);
        }

        String destination = msg.getHeader("destination");
        String subscriptionId = msg.getHeader("id");

        if (subscriptions.containsValue(destination)) {
            return createErrorFrame("User is already subscribed to this channel.", receiptID, msg);
        }

        if (destination == null || subscriptionId == null) {
            return createErrorFrame("SUBSCRIBE frame missing 'destination' or 'id' header.", receiptID, msg);
        }
        // ido added a print
        System.out.println(username + " is subscribing to " + destination.toString());

        subscriptions.put(subscriptionId, destination);
        ((ConnectionsImpl<StompFrame>) connections).subscribeToChannel(destination, connectionId);

        return createReceiptFrame(receiptID);
    }

    private StompFrame handleUnsubscribe(StompFrame msg) {
        String receiptID = msg.getHeader("receipt");
        if (username == null) {
            return createErrorFrame("User not logged in. Please CONNECT first.", receiptID, msg);
        }
        String subscriptionId = msg.getHeader("id");

        if (subscriptionId == null || !subscriptions.containsKey(subscriptionId)) {
            return createErrorFrame("UNSUBSCRIBE frame missing 'id' or invalid subscription.", receiptID, msg);
        }

        String destination = subscriptions.remove(subscriptionId);
        ((ConnectionsImpl<StompFrame>) connections).unsubscribeFromChannel(destination, connectionId);

        // ido added a print
        System.out.println(username + " is unsubscribing from " + destination.toString());

        return createReceiptFrame(receiptID);
    }

    private StompFrame handleSend(StompFrame msg) {
        if (username == null) {
            return createErrorFrame("User not logged in. Please CONNECT first.", null, msg);
        }
        String finalDestination = msg.getHeader("destination");

        if (finalDestination == null) {
            return createErrorFrame("SEND frame missing 'destination' header.", null, msg);
        }

        if (!subscriptions.containsValue(finalDestination)) {
            return createErrorFrame("User is not subscribed to this channel.", null, msg);
        }
        StompFrame returnMsg = new StompFrame("MESSAGE");
        returnMsg.setBody(msg.getBody());
        returnMsg.addHeader("subscription", subscriptions.entrySet().stream()
                .filter(entry -> entry.getValue().equals(finalDestination))
                .map(Map.Entry::getKey)
                .findFirst()
                .orElse(null));
        returnMsg.addHeader("Message-id", String.valueOf(connectionId));
        returnMsg.addHeader("destination", finalDestination);

        System.out.println("User " + username + " sent a message to " + finalDestination);
        // send the meassage to all the subscribers
        ((ConnectionsImpl<StompFrame>) connections).send(finalDestination, returnMsg);

        return createReceiptFrame(msg.getHeader("receipt"));
    }

    private StompFrame handleDisconnect(StompFrame msg) {
        String receiptID = msg.getHeader("receipt");

        if (username == null) {
            return createErrorFrame("User not logged in. Please CONNECT first.", receiptID, msg);
        }

        // Remove the user from active users
        StompFrame s = createReceiptFrame(receiptID);
        System.out.println(s);
        System.out.println("User " + username + " disconnected");
        ((ConnectionsImpl<StompFrame>) connections).send(this.connectionId, s);

        try {
            Thread.sleep(100);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        ((ConnectionsImpl<StompFrame>) connections).logoutUser(username);

        // Clear all subscriptions
        subscriptions.clear();
        
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

    // function to create an error frame, if the frame have receipt id it will add
    // to the error frame, else, it will get null as the receipt id and will not add
    // it to the error frame
    private StompFrame createErrorFrame(String message, String receiptId, StompFrame msg) {
        StompFrame errorFrame = new StompFrame("ERROR");
        if (receiptId != null) {
            errorFrame.addHeader("receipt-id", receiptId);
        }
        errorFrame.addHeader("message", message);
        errorFrame.setBody("The message: \n" + "-----\n" + msg.toString() + "\n-----\n");
        return errorFrame;
    }
}