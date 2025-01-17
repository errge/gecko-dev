/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "ApplicationReputationTelemetryUtils.h"
#include "mozilla/Assertions.h"

using ServerLabel = mozilla::Telemetry::LABELS_APPLICATION_REPUTATION_SERVER_2;

struct NSErrorTelemetryResult {
  nsresult mValue;
  ServerLabel mLabel;
};

static const NSErrorTelemetryResult sResult[] = {
    {
        NS_ERROR_ALREADY_CONNECTED,
        ServerLabel::ErrAlreadyConnected,
    },
    {
        NS_ERROR_NOT_CONNECTED,
        ServerLabel::ErrNotConnected,
    },
    {
        NS_ERROR_CONNECTION_REFUSED,
        ServerLabel::ErrConnectionRefused,
    },
    {
        NS_ERROR_NET_TIMEOUT,
        ServerLabel::ErrNetTimeout,
    },
    {
        NS_ERROR_OFFLINE,
        ServerLabel::ErrOffline,
    },
    {
        NS_ERROR_PORT_ACCESS_NOT_ALLOWED,
        ServerLabel::ErrPortAccess,
    },
    {
        NS_ERROR_NET_RESET,
        ServerLabel::ErrNetReset,
    },
    {
        NS_ERROR_NET_INTERRUPT,
        ServerLabel::ErrNetInterrupt,
    },
    {
        NS_ERROR_PROXY_CONNECTION_REFUSED,
        ServerLabel::ErrProxyConnection,
    },
    {
        NS_ERROR_NET_PARTIAL_TRANSFER,
        ServerLabel::ErrNetPartial,
    },
    {
        NS_ERROR_NET_INADEQUATE_SECURITY,
        ServerLabel::ErrNetInadequate,
    },
    {
        NS_ERROR_UNKNOWN_HOST,
        ServerLabel::ErrUnknownHost,
    },
    {
        NS_ERROR_DNS_LOOKUP_QUEUE_FULL,
        ServerLabel::ErrDNSLookupQueue,
    },
    {
        NS_ERROR_UNKNOWN_PROXY_HOST,
        ServerLabel::ErrUnknownProxyHost,
    },
};

mozilla::Telemetry::LABELS_APPLICATION_REPUTATION_SERVER_2 NSErrorToLabel(
    nsresult rv) {
  MOZ_ASSERT(rv != NS_OK);

  for (const auto& p : sResult) {
    if (p.mValue == rv) {
      return p.mLabel;
    }
  }
  return ServerLabel::ErrOthers;
}

mozilla::Telemetry::LABELS_APPLICATION_REPUTATION_SERVER_2 HTTPStatusToLabel(
    uint32_t status) {
  MOZ_ASSERT(status != 200);

  ServerLabel label;
  switch (status) {
    case 100:
    case 101:
      // Unexpected 1xx return code
      label = ServerLabel::HTTP1xx;
      break;
    case 201:
    case 202:
    case 203:
    case 205:
    case 206:
      // Unexpected 2xx return code
      label = ServerLabel::HTTP2xx;
      break;
    case 204:
      // No Content
      label = ServerLabel::HTTP204;
      break;
    case 300:
    case 301:
    case 302:
    case 303:
    case 304:
    case 305:
    case 307:
    case 308:
      // Unexpected 3xx return code
      label = ServerLabel::HTTP3xx;
      break;
    case 400:
      // Bad Request - The HTTP request was not correctly formed.
      // The client did not provide all required CGI parameters.
      label = ServerLabel::HTTP400;
      break;
    case 401:
    case 402:
    case 405:
    case 406:
    case 407:
    case 409:
    case 410:
    case 411:
    case 412:
    case 414:
    case 415:
    case 416:
    case 417:
    case 421:
    case 426:
    case 428:
    case 429:
    case 431:
    case 451:
      // Unexpected 4xx return code
      label = ServerLabel::HTTP4xx;
      break;
    case 403:
      // Forbidden - The client id is invalid.
      label = ServerLabel::HTTP403;
      break;
    case 404:
      // Not Found
      label = ServerLabel::HTTP404;
      break;
    case 408:
      // Request Timeout
      label = ServerLabel::HTTP408;
      break;
    case 413:
      // Request Entity Too Large
      label = ServerLabel::HTTP413;
      break;
    case 500:
    case 501:
    case 510:
      // Unexpected 5xx return code
      label = ServerLabel::HTTP5xx;
      break;
    case 502:
    case 504:
    case 511:
      // Local network errors, we'll ignore these.
      label = ServerLabel::HTTP502_504_511;
      break;
    case 503:
      // Service Unavailable - The server cannot handle the request.
      // Clients MUST follow the backoff behavior specified in the
      // Request Frequency section.
      label = ServerLabel::HTTP503;
      break;
    case 505:
      // HTTP Version Not Supported - The server CANNOT handle the requested
      // protocol major version.
      label = ServerLabel::HTTP505;
      break;
    default:
      label = ServerLabel::HTTPOthers;
  };
  return label;
}
