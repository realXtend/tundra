framework.Scene().SceneAdded.connect(SetIMSettings);

function SetIMSettings()
{
    /* Parameters for the following method
     * 1. Filtering enabled (true/false)
     * 2. Euclidean distance filter enabled (true/false)
     * 3. Ray Visibility filter enabled (true/false)
     * 4. Relevance filter enabled (true/false)
     * 5. Critical range (int)
     * 6. Raycast range (int)
     * 7. Relevance range (int)
     * 8. Raycast interval ms (int)
     * 9. Update interval ms (int)
    */
    print("Applying Interest Manager settings");
    syncmanager.UpdateInterestManagerSettings(true, true, true, true, 20, 20, 60, 250, 250);
}
