framework.Scene().SceneAdded.connect(SetIMSettings);

function SetIMSettings()
{
    if(framework.GetModuleByName("TundraLogic"))
    {
        var tundralogicmodule = framework.GetModuleByName("TundraLogic")

        /* Parameters for the following method
         * 1. Filtering enabled (true/false)
         * 2. Euclidean distance filter enabled (true/false)
         * 3. Ray Visibility filter enabled (true/false)
         * 4. Relevance filter enabled (true/false)
         * 5. Critical range (int)
         * 6. Raycast range (int)
         * 7. Relevance range (int)
         * 8. Update interval (double)
        */
        print("Applying Interest Manager settings");
        tundralogicmodule.InterestManagerSettingsUpdated(true, true, true, true, 20, 20, 60, 1);
    }
    else
    {
        print("TundraLogicModule was not found, cannot proceed");
    }
}
