package bgu.spl.net.impl.stomp;

import java.util.Map;
import java.util.HashMap;

public class StompFrame {
    private final String command;
    private final Map<String, String> headers;
    private String body;

    public StompFrame(String command, Map<String, String> headers, String body) {
        this.command = command;
        this.headers = headers;
        this.body = body;
    }

    public StompFrame(String command) {
        this(command, new HashMap<>(), "");
    }

    public String getCommand() {
        return command;
    }

    public Map<String, String> getHeaders() {
        return headers;
    }

    public String getBody() {
        return body;
    }

    public String getHeader(String key) {
        return headers.get(key);
    }

    public void addHeader(String key, String value) {
        headers.put(key, value);
    }

    public void removeHeader(String key) {
        headers.remove(key);
    }

    public void setBody(String body) {
        this.body = body;
    }

    public String toString() {
        StringBuilder frame = new StringBuilder();

        // Add the command
        frame.append(command).append("\n");

        // Add headers
        for (Map.Entry<String, String> header : headers.entrySet()) {
            frame.append(header.getKey()).append(":").append(header.getValue()).append("\n");
        }

        // Add a blank line and the body
        frame.append("\n").append(body);

        return frame.toString();
    }
}
