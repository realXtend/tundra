# Synchronization - User and Programmer's Guide

## Introduction

This document describes the use of the Synchronization Generic Enabler.

The typical use of this GE is creating multi-user networked scenes, which may include 3D visualization (for example a virtual world or a multiplayer game). A client connects, using the WebSocket protocol, to a server where the multi-user scene is hosted, after which they will receive the scene content and any updates to it, for example objects' position updates under a physics simulation calculated by the server. The clients interact with the scene by either directly manipulating the scene content (this may not be feasible in all cases and can be prevented by security scripts running on the server) or by sending Entity Actions, which resemble remote procedure calls, that can be interpreted by scripts running on the server. For example, the movement controls of a client controlled character ("avatar") could be sent as Entity Actions (ie. move forward, stop moving forward, rotate 45 degrees right.)

Another use case is using the SceneAPI REST service to do infrequent scene queries and modifications. This has the advantage of not having to use the synchronization client library, and to not tie system resources to maintaining a real-time connection on either the server or the client.

Note that the Synchronization GE client code itself does not visualize anything, it only updates the internal scene data model according to data from the network. However, the same WebTundra codebase that houses the Synchronization client code also contains an implementation of the 3D-UI GE, which implements 3D visualization on top of the scene data model.

## User guide

The user guide section describes on a high level the data model used by this GE, and how scenes are loaded into the server.

### Data model

The data that is transmitted between the server and the client is based on the Entity-Component-Attribute model as used by realXtend Tundra. It is described here [https://github.com/realXtend/tundra/wiki/Scene-and-EC-Model](https://github.com/realXtend/tundra/wiki/Scene-and-EC-Model)

In particular, the following actions between the server and the client are synchronized bidirectionally through the WebSocket connection:

- Create an entity
- Remove an entity
- Create a component into an entity
- Remove a component from an entity
- Modify an attribute value in a component. This is the bulk of the network traffic. For example when an entity moves, the Transform attribute of its Placeable component is being updated constantly.
- Create an attribute into the DynamicComponent type of component.
- Remove an attribute from a DynamicComponent.
- Send an Entity Action.

When multiple clients are connected, such modifications are sent to all of them.

The SceneAPI REST service offers an XML-based view into the scene, and provides the same operations as the real-time synchronization protocol, except for not providing Entity Actions, which are of a real-time nature:

- Query for all entities in the scene
- Query for an entity, component, or attribute
- Create an entity, with initial data or not
- Remove an entity
- Create a component into an entity, with initial data or not
- Remove a component from an entity
- Modify one or more attribute values in a component.

### Using the Tundra server

The realXtend Tundra SDK is a complex piece of software and to provide a full description of how it can be used is beyond the scope of this document. For more documentation please see [http://doc.meshmoon.com/](http://doc.meshmoon.com/) Note that MeshMoon is a proprietary hosting solution based on Tundra SDK but it currently provides the best, most up-to-date documentation of the Tundra SDK. Parts of the documentation which pertain only to the MeshMoon specific extensions and not the core Tundra SDK itself are marked so.

To see all Tundra's command line options use the command

<pre>
Tundra --help
</pre>

The following command line options are often used:

<pre>
--file scenename.txml  Specify the txml scene file to open on startup. Example scenes exist 
                       in the bin/scenes directory.
--server               Run as a server
--port portnumber      Specify which port the server listens on. This is both for native
                       clients (UDP protocol) and WebSocket clients (TCP protocol)
--headless             Run without graphics rendering
--httpport portnumber  Enable Tundra HTTP server, which provides the SceneAPI REST service. 
                       This requires the HttpServerModule to be loaded
--config configfile    Specify the JSON configuration file(s) to use. If not specified, the 
                       default configuration file tundra.json is used.
</pre>

### Example

Without going into actual programming, the scene data synchronization functionality can be demonstrated in the following way using the Tundra server and a web browser. This assumes that both the Tundra server and the client JavaScript library source code have been successfully installed on the same machine as described in the Installation and Administration Guide.

Start up the Tundra server with rendering enabled so that you can see the scene. Here we choose the Physics2 example scene which consists of a large number of physics objects and a ball suspended above them. The server mode is enabled so that clients can connect. Additionally the tundra-addons.json configuration file is loaded, which allows the HttpServerModule (implements the SceneAPI REST service) to be loaded. The SceneAPI REST service is bound to port 2346.

<pre>
Tundra --config tundra.json --config tundra-addons.json --file scenes/Physics2/scene.txml --server --httpport 2346
</pre>

Note: if running the server from prebuilt binaries, the --config command line options can be omitted, as the tundra-addons.json configuration file is already being run by default.

After this, open the HTML document examples/Physics2/index.html with a web browser from the root directory of the client source code. Connect to the server by clicking the "Connect" button. Verify that you see the same scene also in the browser window, and that the moving light object is seen moving.

Now, click on the suspended ball above the objects in the server window. Observe that the movement of the ball and the colliding objects are replicated to the browser window as well.

In a similar manner, the SceneAPI REST service can be demonstrated. The service requests use a URL path starting with /scene or /entities. The following GET request (can be executed with a browser) when the server is running, should display the whole scene's contents in XML format:

<pre>
http://localhost:2346/entities
</pre>

## Programmers guide

The programmer's guide describes the JavaScript client library API, the SceneAPI REST service methods, and relevant parts of the server code's operation, as well as the binary-level protocol used.

### JavaScript client library

The Synchronization client code is housed in the WebTundra code base [https://github.com/realxtend/webtundra](https://github.com/realxtend/webtundra). See its documentation for full details. Some simple example how to run WebTundra is described here:

#### From binary ####

Download latest WebTundra package from [WebTundra Catalog](http://catalogue.fiware.org/enablers/3dui-webtundra/downloads) and extract the zip file

Startup the Synchronization server:

<pre>TundraConsole --server --headless --file scenes/Physics/scene.txml</pre>

Start `apache2` or `node.js http-server` in where you extracted the zip file and open `client.html` in web browser and press connect.

#### From source ####

Clone WebTundra from GitHub:

<pre>git clone https://github.com/realXtend/WebTundra.git</pre>

Install [`node.js`](http://nodejs.org/) and [`grunt`](http://gruntjs.com/) for making build:
<pre>
1. Install [node.js](http://nodejs.org/)
2. Run `npm install` on the repo root folder to fetch dependencies.
3. Run `npm install -g grunt-cli` to install the grunt executable as a global tool.
</pre>

Run [`grunt`](http://gruntjs.com/) in WebTundra root folder:

<pre>grunt build</pre>

Startup the Synchronization server:

<pre>TundraConsole --server --headless --file scenes/Physics/scene.txml</pre>

Start `apache2` or `node.js http-server` in `WebTundra/build` folder and open `client.html` in web browser and press connect.

### SceneAPI REST service

When the Tundra server has been started with the SceneAPI REST service enabled (HttpServerModule is loaded and the --httpport command line parameter has been given to set up the port it should be bound to) the following requests are available. Note that /entities is interchangeable with /scene.

#### Queries

Query all entities in the scene. They are returned as XML data with a root 'scene' element similar to the .txml scene files

<pre>
GET /entities
</pre>

Query a specific entity from the scene by ID number. It is returned as XML data with a root 'entity' element.

<pre>
GET /entities/id
</pre>

Query a specific entity from the scene by its name. It is returned as XML data with a root 'entity' element.

<pre>
GET /entities?name=EntityName
</pre>

Query a specific component type (for example Mesh or Placeable) from an entity specified by ID. The component is returned as XML data with a root 'component' element.

<pre>
GET /entities/id/componentTypeName
</pre>

Query a specific attribute from a component from an entity specified by ID. The attribute value is returned as plaintext. For example /entities/1/placeable/transform

<pre>
GET /entities/id/componentTypeName/attributeName
</pre>

#### Scene manipulation

Replace an entity's data. Existing components will be removed. The data should be contained in the request body in XML format with a root 'entity' element. The updated entity XML data is sent back.

<pre>
PUT /entities/id
</pre>

Replace a component's attribute data. The attribute data should be contained in the request body in XML format with a root 'component' element. The updated component XML data is sent back.

<pre>
PUT /entities/id/componentTypeName
</pre>

Replace attribute value(s) in a component using query syntax. The updated component XML data is sent back.

<pre>
PUT /entities/id/componentTypeName?attributeName1=newValue1&attributeName2=newValue2
</pre>

Create a new entity while letting the server assign an ID. The entity's component data can optionally be contained in the request body in XML format with a root 'entity' element. The new entity's XML data (showing the proper server-assigned ID) is sent back.

<pre>
POST /entities
</pre>

Create a new entity with specific ID. If an entity with that ID already exists, the server will assign a new ID. The entity's component data can optionally be contained in the request body in XML format with a root 'entity' element. The new entity's XML data (showing the proper server-assigned ID) is sent back.

<pre>
POST /entities/id
</pre>

Create a new component to an entity. The component's data can optionally be contained in the request body in XML format with a root 'component' element. The new component's XML data is sent back.

<pre>
POST /entities/id/componentTypeName
</pre>

Remove an entity from the scene by ID.

<pre>
DELETE /entities/id
</pre>

Remove an entity from the scene by name.

<pre>
DELETE /entities?name=EntityName
</pre>

Remove a component from an entity.

<pre>
DELETE /entities/id/componentTypeName
</pre>

### Server plugins

The server functionality of the Synchronization GE is implemented as two Tundra modules.

#### WebSocketServerModule

WebSocketServerModule implements the real-time synchronization protocol. Its code can be viewed here: [https://github.com/realXtend/tundra/tree/tundra2/src/Application/WebSocketServerModule](https://github.com/realXtend/tundra/tree/tundra2/src/Application/WebSocketServerModule)

It uses the websocketpp library for implementing WebSocket communications [https://github.com/zaphoyd/websocketpp](https://github.com/zaphoyd/websocketpp)

The WebSocketServerModule registers the WebSocket client connections to the Tundra main server class, so that other server-side modules and scripts can treat native (C++ client) and Web client connections as equivalent. The client list including both native and Web client connections can be received with one function call:

<pre>
framework->GetModule<TundraLogic::TundraLogicModule>()->GetServer()->AuthenticatedUsers();
</pre>

In server-side JavaScript code, this would be respectively:

<pre>
server.AuthenticatedUsers();
</pre>

#### HttpServerModule

HttpServerModule implements the SceneAPI REST service. Its code can be viewed as a part of the TundraAddons git repository: [https://github.com/realXtend/TundraAddons/tree/master/HttpServerModule](https://github.com/realXtend/TundraAddons/tree/master/HttpServerModule)

This module also uses websocketpp library for the HTTP communications, though it is a modified version.

The HttpServerModule handles by itself requests that begin with URL path /scene or /entities. It does not handle other requests; instead these are emitted as a Qt signal by the HttpServer object; other C++ code can connect to this signal. To acquire the HttpServer object, use the following line of code:

<pre>
framework->GetModule<HttpServerModule>()->GetServer();
</pre>

See the file [https://github.com/realXtend/TundraAddons/tree/master/HttpServerModule/HttpServer.h](https://github.com/realXtend/TundraAddons/tree/master/HttpServerModule/HttpServer.h) for the signal definition.

### Synchronization binary protocol

For the description of the byte-level protocol see [https://github.com/realXtend/tundra/wiki/Tundra-protocol](https://github.com/realXtend/tundra/wiki/Tundra-protocol).

- Each message is sent as one binary WebSocket frame, with the message ID encoded as an unsigned little-endian 16-bit value in the beginning.
- Login data (message ID 100) is JSON instead of XML.
- Before the server starts sending scene messages, the client must "authenticate" itself by sending the login message. In a default Tundra server configuration (no scene password, no security scripts) the actual data content sent in the login message does not matter.