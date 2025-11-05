/**
 * @file backend_server_secrets_example.h
 * @brief Example backend server configuration and API key placeholders.
 *
 * This file provides example values for `BACKEND_SERVER_URL` and
 * `BACKEND_API_KEY` so the project can compile out-of-the-box. DO NOT
 * commit real secrets into source control. Instead, copy this file to
 * `backend_server_secrets.h` locally and populate it with live values.
 *
 * Example:
 *  cp backend_server_secrets_example.h backend_server_secrets.h
 *  // edit backend_server_secrets.h to set real BACKEND_SERVER_URL and BACKEND_API_KEY
 *
 * The code will prefer `backend_server_secrets.h` if it exists; otherwise the
 * example values here are used as a fallback for testing.
 */
#ifndef BACKEND_SERVER_SECRETS_EXAMPLE_H
#define BACKEND_SERVER_SECRETS_EXAMPLE_H

/** Backend server URL used for HTTP POST requests (example placeholder) */
#define BACKEND_SERVER_URL "https://your-backend-server.com"

/** Backend API key placeholder - replace in local backend_server_secrets.h */
#define BACKEND_API_KEY "your_api_key"

#endif // BACKEND_SERVER_SECRETS_EXAMPLE_H
