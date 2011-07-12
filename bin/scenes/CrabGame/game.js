//Control for gameObject on Server side
function ServerObjectControl (blockId, name)
{
    this.sName = name;
    this.sBlockId = blockId;
    this.ServerObject = scene.GetEntityRaw(blockId);
    print(this.sName);
    this.visible = true;
    this.moveX = 0.5;
    this.moveY = 0.5;
}

ServerObjectControl.prototype.connectBlock = function()
{
    if (!this.ServerObject)
        print("Missing " + this.sName + " with ID " + this.sBlockId);
        
    else
    {    
        var mode = this.ServerObject.placeable.GetUpdateMode();
        mode.value = 3;
        this.ServerObject.placeable.SetUpdateMode(mode);
        
        this.ServerObject.Exec(4, "StopAnim", "stand", 0.5);        
        this.ServerObject.Exec(4, "PlayAnim", "walk", 0.5);

        this.ServerObject.Action("PlayerMousePress").Triggered.connect(this, this.blockClicked);
    }
    
    frame.Updated.connect(this, this.Update);    
}

ServerObjectControl.prototype.Update = function(frametime)
{
    if(this.visible)
    {        
        var trans = this.ServerObject.placeable.transform;
        
        //Check if objects are on defined game-area, change direction if not
        if(trans.pos.x >= -2.0)
            this.moveX = -0.5;

        if(trans.pos.x <= -11.0)
            this.moveX = 0.5;
        
        if(trans.pos.y >= -3.0)
            this.moveY = -0.5;
    
        if(trans.pos.y <= -11.0)
            this.moveY = 0.5;
        
        trans.pos.x += this.moveX * frametime;
        trans.pos.y += this.moveY * frametime;        
        this.ServerObject.placeable.transform = trans;
    }

}

ServerObjectControl.prototype.blockClicked = function(fromPlayer)
{ 
    if(this.visible)
    {        
        this.hide();
        
        //adding the points
        if (fromPlayer == 1)
        {
            points = compGameStats.GetAttribute("Player1Points");
            points += 1.0;
            compGameStats.SetAttribute("Player1Points", + points);    
        }
        
        else if (fromPlayer == 2)
        {         
            points = compGameStats.GetAttribute("Player2Points");
            points += 1.0;
            compGameStats.SetAttribute("Player2Points", + points);
        }
    }    
}

ServerObjectControl.prototype.show = function()
{
    this.ServerObject.Exec(4, "ShowEntity");
    
    //new random position for object on game area
    var trans = this.ServerObject.placeable.transform;
    trans.pos.x = -2.0 - Math.floor(Math.random()*9);
    trans.pos.y = -2.0 - Math.floor(Math.random()*9);
    trans.pos.z = 77.0;
    this.ServerObject.placeable.transform = trans;    
    this.visible = true;
}
    
ServerObjectControl.prototype.hide = function()
{    
    this.visible = false;
    this.ServerObject.Exec(4, "HideEntity");
    randTime = 1.0 + Math.floor(Math.random() * 4); // random time in secs for entity to be hidden

    var trans = this.ServerObject.placeable.transform;
    trans.pos.z = 70.0;
    this.ServerObject.placeable.transform = trans;    
    
    delayed = frame.DelayedExecute(randTime);
    delayed.Triggered.connect(this, this.show);
}

ServerObjectControl.prototype.ReturnToOrig = function()
{
    this.ServerObject.Exec(4, "StopAnim", "walk", 0.5);        
    this.ServerObject.Exec(4, "PlayAnim", "stand", 0.5);
    frame.Updated.disconnect(this, this.Update);
}

//Control for gameObject on Client side
function Block (blockId, name)
{
    this.name = name;
    this.BlockId = blockId;
    this.EntBlock = scene.GetEntityRaw(blockId);
}

Block.prototype.connect = function()
{
    if (!this.EntBlock)
        print("Missing " + this.name + " with ID " + this.BlockId);
        
    else
    {    
        this.EntBlock.Action("MousePress").Triggered.connect(this, this.playerBlockClicked);
        this.EntBlock.Action("PlayerMousePress").Triggered.connect(this.blockClicked);
    }
        
}
Block.prototype.blockClicked = function()
{
//Empty function for testing
//This is here only because of PlayerMousePress action
//PlayerMousePress action is executed on client and triggered on server. 
//At the moment it doesn't work right, if it is not triggered on client side too
}

Block.prototype.playerBlockClicked = function()
{ 
    this.EntBlock.Exec(2, "PlayerMousePress", player);
    print("player click " + player + " " + this.name);    
}    

print("Loading TestGame.");

//Gamestats
var compGameStats = me.GetComponent("EC_DynamicComponent");
var mode = compGameStats.GetUpdateMode();
print(mode.value);
mode.value = 3;
compGameStats.SetUpdateMode(mode);

server = me.GetComponent("EC_DynamicComponent", "server");

if (server)
{
    print("Server Creating and connecting to GameBlocks...");
    var sBlock1 = new ServerObjectControl(181, "Crab1");
    var sBlock2 = new ServerObjectControl(183, "Crab2");
    var sBlock3 = new ServerObjectControl(184, "Crab3");
    var sBlock4 = new ServerObjectControl(185, "Crab4");
    
    sBlock1.connectBlock();
    sBlock2.connectBlock();
    sBlock3.connectBlock();
    sBlock4.connectBlock();
}
else
{
    //Game will start when clicked the entity where the script is, and end when clicked grass_ground_plane04 entity (ID 180)
    me.Action("MousePress").Triggered.connect(StartGame);
    var closingEntity = scene.GetEntityRaw(180);

    //Creating controls Objects
    var pBlock1 = new Block(181, "Crab1");
    var pBlock2 = new Block(183, "Crab2");
    var pBlock3 = new Block(184, "Crab3");
    var pBlock4 = new Block(185, "Crab4");
    
    var player = 0;
    var playerName;

    //ui for points
    // Load ui file and attach it to proxywidget.
    var widget = ui.LoadFromFile(".\\jsmodules\\apitest\\CrabGame\\gamestats.ui", false);
    var lcdPoints = widget.findChild("lcdNumber");
    lcdPoints.value = 0.0;
    var lblPlayer1 = widget.findChild("lblPlayer1");
    lblPlayer1.text = "empty";
    var lblPlayer2 = widget.findChild("lblPlayer2");
    lblPlayer2.text = "empty";
    var lcdOpponent = widget.findChild("lcdNumber_2");
    lcdOpponent.value = 0.0;
    var proxy = new UiProxyWidget(widget);
    ui.AddWidgetToScene(proxy);
    proxy.x = 600;
    proxy.y = 50;
    proxy.visible = false;
    proxy.windowFlags = 0;
}

function StartGame()
{
    print("Starting Game..");
    proxy.visible = true;
    var playerName;
    
    me.Action("MousePress").Triggered.disconnect(StartGame);
    closingEntity.Action("MousePress").Triggered.connect(EndGame);
    
    var player1 = compGameStats.GetAttribute("Player1");
    var player2 = compGameStats.GetAttribute("Player2");    

    if (player1 == "empty")
    {    player = 1;
        playerName = "Player 1 from Script";
        lblPlayer1.text = "Your Points ";    
        compGameStats.SetAttribute("Player1", playerName);
        compGameStats.SetAttribute("Player1Points", 0.0);        
    }
    
    else if (player2 == "empty")
    {
        player = 2;
        playerName = "Player 2 from Script";
        lblPlayer1.text = "Your Points ";
        compGameStats.SetAttribute("Player2", playerName);
        compGameStats.SetAttribute("Player2Points", 0.0);        
    }
    
    print(playerName);
        
    lcdPoints.value = 0.0;
    lcdOpponent.value = 0.0;
    
    pBlock1.connect();
    pBlock2.connect();
    pBlock3.connect();
    pBlock4.connect();
    
    //Connect to attribute change for getting the game stats
    scene.AttributeChanged.connect(PrintIt);
    
}

function EndGame()
{
    if (server)
    {
        sBlock1.ReturnToOrig();
        sBlock2.ReturnToOrig();
         sBlock3.ReturnToOrig();
        sBlock4.ReturnToOrig();
    }
    
    else
    {
        closingEntity.Action("MousePress").Triggered.disconnect(EndGame);
        me.Action("MousePress").Triggered.connect(StartGame);    
    
        if (player == 1)
            compGameStats.SetAttribute("Player1", "empty");
        else if (player == 2)
            compGameStats.SetAttribute("Player2", "empty");
        
        player = 0;
    
        scene.AttributeChanged.disconnect(PrintIt);
        lcdPoints.value = 0.0;
        lcdOpponent.value = 0.0;
    
        proxy.visible = false;
    }
}

function PrintIt(component)
{
    print(component);
    if(component == "EC_DynamicComponent(name = \"\")")
    
    if(player == 1)
    {
        lcdPoints.intValue = compGameStats.GetAttribute("Player1Points");
        lblPlayer2.text = compGameStats.GetAttribute("Player2");
        lcdOpponent.intValue = compGameStats.GetAttribute("Player2Points");
    }
    
    else if (player == 2)
    {
        lcdPoints.intValue = compGameStats.GetAttribute("Player2Points");
        lblPlayer2.text = compGameStats.GetAttribute("Player1");
        lcdOpponent.intValue = compGameStats.GetAttribute("Player1Points");
    }
}

function OnScriptDestroyed()
{    
    EndGame();
}
