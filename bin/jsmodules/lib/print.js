function printObj(obj) {
  for (i in obj) {
    if (i == "locale") // This is a hack to avoid calling QLocale::toString in iteration, which would throw an overload resolution error inside Qt.
      print(i + ": locale"); 
    else
      print(i + ": " + obj[i]);
  }
}                     