using System.Xml;

public class Casts {
    public static void Main() {
        var x = new XmlDocument();
//      var y = (XmlDocument)"abc";
        var z = (object)x;
        var w = (XmlDocument)z;
        var s = "abc";
//      var t = (XmlDocument)s;
//      var u = s as XmlDocument;
    }
}
