// IntegerCheck.js - For validating that C++ integer typedefs work properly with QtScript.

function log(msg)
{
    console.LogInfo("[Tests::IntegerCheck]: " + msg);
}

function onProperty(num)
{
    log("JS read Q_PROPERTY with " + num + " (" + typeof num + ")");
}

function onSignal(num)
{
    log("JS received signal with " + num + " (" + typeof num + ")");
}

print("");

var runner = new IntegerTestRunner();

// Connect signals
runner.onSizeT.connect(onSignal);
runner.onEnitityId.connect(onSignal);
runner.onComponentId.connect(onSignal);
runner.onS8.connect(onSignal);
runner.onU8.connect(onSignal);
runner.onS16.connect(onSignal);
runner.onU16.connect(onSignal);
runner.onS32.connect(onSignal);
runner.onU32.connect(onSignal);
runner.onS64.connect(onSignal);
runner.onU64.connect(onSignal);

print("");

// Qt properties
onProperty(runner.propSizeT);
onProperty(runner.propEntityId);
onProperty(runner.propComponentId);
onProperty(runner.propS8);
onProperty(runner.propU8);
onProperty(runner.propS16);
onProperty(runner.propU16);
onProperty(runner.propS32);
onProperty(runner.propU32);
onProperty(runner.propS64);
onProperty(runner.propU64);

print("");

// Use getters to call setters 
// which emit the above signals
runner.setSizeT(runner.getSizeT());
runner.setEntityId(runner.getEntityId());
runner.setComponentId(runner.getComponentId());
runner.setS8(runner.getS8());
runner.setU8(runner.getU8());
runner.setS16(runner.getS16());
runner.setU16(runner.getU16());
runner.setS32(runner.getS32());
runner.setU32(runner.getU32());
runner.setS64(runner.getS64());
runner.setU64(runner.getU64());

print("");

// Overflows
log("Overflowing s8 with 128:");
runner.setS8(128);
log("Overflowing s16 with 32768:");
runner.setS16(32768);
log("Overflowing s32 with 2147483648:");
runner.setS32(2147483648);

print("");

runner.deleteLater();
runner = null;
