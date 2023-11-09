#include <libwebsockets.h>
#include <stdio.h>
#include <string.h>

static int callback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len) {
    switch (reason) {
        case LWS_CALLBACK_CLIENT_ESTABLISHED:
            printf("Connected to the WebSocket server...\n");
            // Send your request here, for example:
            lws_callback_on_writable(wsi);
            break;

        case LWS_CALLBACK_CLIENT_RECEIVE:
            if (len > 0) {
                char receivedData[len + 1];
                memset(receivedData, 0, sizeof(receivedData));
                memcpy(receivedData, in, len);
                printf("Received data: %s\n", receivedData);
            }
            break;

        case LWS_CALLBACK_CLIENT_WRITEABLE:
            // Send your request here
            lws_write(wsi, "Your WebSocket request data", strlen("Your WebSocket request data"), LWS_WRITE_TEXT);
            break;

        case LWS_CALLBACK_CLOSED:
            printf("WebSocket connection closed.\n");
            break;

        default:
            break;
    }

    return 0;
}

int main(void) {
    struct lws_context_creation_info info;
    struct lws_client_connect_info i;
    struct lws_context *context;
    const char *address = "webradio.antenne.de/api/metadata/now";  // Replace with the WebSocket server URL

    memset(&info, 0, sizeof info);
    info.port = CONTEXT_PORT_NO_LISTEN;
    info.protocols = lws_protocols;
    info.ssl_ca_filepath = "/etc/ssl/certs"; // Adjust for your system
    info.options = LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;

    context = lws_create_context(&info);
    if (!context) {
        fprintf(stderr, "libwebsocket init failed\n");
        return -1;
    }

    memset(&i, 0, sizeof i);
    i.context = context;
    i.address = address;
    i.port = 443;
    i.path = "/";
    i.host = lws_canonical_hostname(context);
    i.origin = address;
    i.protocol = "wss";

    lws_client_connect_via_info(&i);

    while (1) {
        lws_service(context, 0);
    }

    return 0;
}
