/**
 * @file zcl.identify.h
 * @brief ZCL Identify cluster header
 * ZCL 7 section 3.5
 * ZCL 8 section 3.5
 * @copyright Copyright [2009 - 2020] Exegin Technologies Limited. All rights reserved.
 */

#ifndef ZCL_CORE_IDENTIFY_H
# define ZCL_CORE_IDENTIFY_H

/*--------------------------------------------------------------------------
 *  DESCRIPTION
 *      Interface definition for the ZCL Identify cluster.
 *--------------------------------------------------------------------------
 */

/* PICS.ZCL.Identify Cluster (0x0003)
 *
 * Server PICS
 * I.S | True
 * I.C | True
 *
 * Server Attributes
 * I.S.A0000 | True
 * I.S.Afffd | True
 *
 * Commands Received
 * I.S.C00.Rsp | True
 * I.S.C01.Rsp | True
 * I.S.C40.Rsp | False
 *
 * Commands Generated
 * I.S.C00.Tx | True
 *
 * Client Attributes
 * I.C.Afffd | True
 *
 * Commands Received
 * I.C.C00.Rsp | True
 *
 * Commands Generated
 * I.C.C00.Tx | True
 * I.C.C01.Tx | True
 * I.C.C40.Tx | False
 */

#include "zcl/zcl.h"

/* Identify Server Attribute IDs */
enum ZbZclIdentifySvrAttrT {
    ZCL_IDENTIFY_ATTR_TIME = 0x0000
};

/* Identify Command IDs  */
enum ZbZclIdentifyClientCommandT {
    ZCL_IDENTIFY_COMMAND_IDENTIFY = 0x00,
    ZCL_IDENTIFY_COMMAND_QUERY = 0x01
};

enum ZbZclIdentifyServerCommandT {
    ZCL_IDENTIFY_COMMAND_QUERY_RESP = 0x00
};

/*---------------------------------------------------------------
 * Identify Server Cluster
 *---------------------------------------------------------------
 */

enum ZbZclIdentifyServerStateT {
    ZCL_IDENTIFY_START = 0,
    ZCL_IDENTIFY_STOP
};

/** Callback to receive events to "start" or "stop" identifying. */
typedef void (*ZbZclIdentifyCallbackT)(struct ZbZclClusterT *cluster,
    enum ZbZclIdentifyServerStateT state, void *arg);

/**
 * Instantiate a new instance of the Identify server cluster.
 * @param zb Zigbee stack instance.
 * @param endpoint APS endpoint to create cluster on.
 * @param arg Application argument that gets assigned to ZbZclClusterSetCallbackArg.
 * @return Cluster pointer, or NULL if there as an error.
 */
struct ZbZclClusterT * ZbZclIdentifyServerAlloc(struct ZigBeeT *zb, uint8_t endpoint, void *arg);

/**
 * Set the callback in the cluster private structure
 * @param cluster Cluster instance to send this command from
 * @param callback Callback function to call with command response
 */
void ZbZclIdentifyServerSetCallback(struct ZbZclClusterT *cluster,
    ZbZclIdentifyCallbackT callback);

/**
 * Set the local identify server time. Same as writing to the
 * ZCL_IDENTIFY_ATTR_TIME attribute.
 *
 * If BDB_COMMISSION_MODE_FIND_BIND is enabled and seconds > 0, seconds
 * is adjusted to be >= ZB_BDBC_MinCommissioningTime.
 * @param clusterPtr Zigbee cluster instance.
 * @param seconds Seconds for updating the identify time counter
 */
void ZbZclIdentifyServerSetTime(struct ZbZclClusterT *cluster, uint16_t seconds);

/**
 * Get the local identify server time.
 * @param clusterPtr Zigbee cluster instance.
 * @return uint16_t Time remaining in zigbee timer
 */
uint16_t ZbZclIdentifyServerGetTime(struct ZbZclClusterT *cluster);

/*---------------------------------------------------------------
 * Identify Client Cluster
 *---------------------------------------------------------------
 */

/**
 * Instantiate a new instance of the Identify client cluster.
 * @param zb Zigbee stack instance.
 * @param endpoint APS endpoint to create cluster on.
 * @return Cluster pointer, or NULL if there as an error.
 */
struct ZbZclClusterT * ZbZclIdentifyClientAlloc(struct ZigBeeT *zb, uint8_t endpoint);

/**
 * Start or stop the receiving device identifying itself
 * @param cluster Cluster instance to send this command from
 * @param dst The destination address information
 * @param identify_time Time which will be used to set the IdentifyTime attribute
 * @param callback Callback function to call with command response
 * @param arg Application argument that will be included with the callback
 * @return ZCL_STATUS_SUCCESS if successful, or other ZclStatusCodeT value on error
 */
enum ZclStatusCodeT zcl_identify_identify_request(struct ZbZclClusterT *cluster, const struct ZbApsAddrT *dst,
    uint16_t identify_time, void (*callback)(struct ZbZclCommandRspT *rsp, void *arg), void *arg);

/**
 * Request target(s) to respond if they are currently identifying themselves
 * @param cluster Cluster instance to send this command from
 * @param dst The destination address information
 * @param callback Callback function to call with command response
 * @param arg Application argument that will be included with the callback
 * @return ZCL_STATUS_SUCCESS if successful, or other ZclStatusCodeT value on error
 */
enum ZclStatusCodeT zcl_identify_query_request(struct ZbZclClusterT *cluster, const struct ZbApsAddrT *dst,
    void (*callback)(struct ZbZclCommandRspT *rsp, void *arg), void *arg);

#endif /* __ZCL_IDENTIFY_H */
