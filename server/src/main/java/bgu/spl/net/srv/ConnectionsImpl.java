package bgu.spl.net.srv;

import java.io.IOException;
import java.util.List;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.CopyOnWriteArrayList;

public class ConnectionsImpl<T> implements Connections<T> {
    private ConcurrentHashMap<Integer, ConnectionHandler<T>> connections;
    private ConcurrentHashMap<String, List<Integer>> channels;
    private ConcurrentHashMap<String, String> userCredentials; // Stores username-password pairs
    private ConcurrentHashMap<String, Integer> activeUsers; // Maps username to connectionId

    public ConnectionsImpl() {
        connections = new ConcurrentHashMap<>();
        channels = new ConcurrentHashMap<>();
        userCredentials = new ConcurrentHashMap<>();
        activeUsers = new ConcurrentHashMap<>();
    }

    @Override
    public boolean send(int connectionId, T msg) {
        ConnectionHandler<T> connection = connections.get(connectionId);
        if (connection != null) {
            connection.send(msg);
            return true;
        }
        return false;
    }

    @Override
    public void send(String channel, T msg) {
        List<Integer> subscribers = channels.get(channel);
        if (subscribers != null) {
            for (Integer connectionId : subscribers) {
                ConnectionHandler<T> connection = connections.get(connectionId);
                if (connection != null) {
                    connection.send(msg);
                }
            }
        }
    }

    public void broadcast(T msg) {
        for (ConnectionHandler<T> connection : connections.values()) {
            connection.send(msg);
        }
    }

    @Override
    public void disconnect(int connectionId) {
        connections.remove(connectionId);
        for (List<Integer> subscribers : channels.values()) {
            subscribers.remove(connectionId);
        }

        // Remove the user from activeUsers if connected
        activeUsers.entrySet().removeIf(entry -> entry.getValue().equals(connectionId));
    }

    public void addConnection(int connectionId, ConnectionHandler<T> handler) {
        connections.put(connectionId, handler);
    }

    public void subscribeToChannel(String channel, int connectionId) {
        channels.putIfAbsent(channel, new CopyOnWriteArrayList<>());
        channels.get(channel).add(connectionId);
    }

    public void unsubscribeFromChannel(String channel, int connectionId) {
        List<Integer> subscribers = channels.get(channel);
        if (subscribers != null) {
            subscribers.remove(Integer.valueOf(connectionId));
        }
    }

    // User management methods
    public boolean registerUser(String username, String password) {
        return userCredentials.putIfAbsent(username, password) == null;
    }

    public boolean authenticateUser(String username, String password) {
        return password.equals(userCredentials.get(username));
    }

    public boolean isUserLoggedIn(String username) {
        return activeUsers.containsKey(username);
    }

    public boolean loginUser(String username, int connectionId) {
        if (activeUsers.putIfAbsent(username, connectionId) == null) {
            return true;
        }
        return false; // User is already logged in
    }

    public void logoutUser(String username) {

        Integer connectionId = activeUsers.remove(username);
        if (connectionId != null) {
            // Remove the user from all channels
            for (List<Integer> subscribers : channels.values()) {
                subscribers.remove(connectionId);
            }
            // Close the connection
            ConnectionHandler<T> connection = connections.remove(connectionId);
            if (connection != null) {
                try {
                    connection.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
    }

    public Integer getConnectionId(String username) {
        return activeUsers.get(username);
    }

}
