//package bgu.spl.net.api;
package bgu.spl.net.impl.stomp;

import bgu.spl.net.api.MessageEncoderDecoder;
import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Map;

public class StompEncoderDecoder implements MessageEncoderDecoder<StompFrame> {
    private StringBuilder buffer = new StringBuilder();

    @Override
    public StompFrame decodeNextByte(byte nextByte) {
        if (nextByte == '\0') { // Null character indicates the end of a frame
            String frameData = buffer.toString();
            buffer.setLength(0); // Clear the buffer for the next frame
            return parseFrame(frameData); // Parse the frame
        } else {
            buffer.append((char) nextByte); // Append the byte to the buffer
            return null; // Frame is not complete yet
        }
    }

    @Override
    public byte[] encode(StompFrame message) {
        StringBuilder encodedFrame = new StringBuilder();

        // Add the command
        encodedFrame.append(message.getCommand()).append("\n");

        // Add headers
        for (Map.Entry<String, String> header : message.getHeaders().entrySet()) {
            encodedFrame.append(header.getKey()).append(":").append(header.getValue()).append("\n");
        }

        // Add a blank line and the body
        encodedFrame.append("\n").append(message.getBody());

        // Terminate with the null character
        encodedFrame.append('\0');

        return encodedFrame.toString().getBytes(StandardCharsets.UTF_8);
    }

    // Helper method to parse a raw STOMP frame string into a STOMPFrame object
    private StompFrame parseFrame(String frameData) {
        String[] lines = frameData.split("\n");
        String command = lines[0];

        Map<String, String> headers = new HashMap<>();
        int i = 1;

        // Parse headers until an empty line
        while (i < lines.length && !lines[i].isEmpty()) {
            String[] headerParts = lines[i].split(":", 2);
            headers.put(headerParts[0], headerParts[1]);
            i++;
        }

        // Parse body (if exists)
        StringBuilder body = new StringBuilder();
        i++; // Skip the empty line
        while (i < lines.length) {
            body.append(lines[i]).append("\n");
            i++;
        }

        return new StompFrame(command, headers, body.toString().trim());
    }
}
