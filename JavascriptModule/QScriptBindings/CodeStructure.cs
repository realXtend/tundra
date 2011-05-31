using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Xml;
using System.IO;
using System.Web;
//using System.Windows.Forms;
//using mshtml;
using System.Text.RegularExpressions;

namespace DocGenerator
{
    public enum VisibilityLevel
    {
        Public,
        Protected,
        Private
    };

    public enum Virtualness
    {
        None,
        Virtual,
        Pure
    };

    /// <summary>
    /// "int foo" or "const char *str"
    /// </summary>
    public class Parameter
    {
        public string type;
        public string name;
        public string defaultValue;
        public string comment;

        /// <summary>
        /// If type "const float3 &" -> BasicType() returns "float3".
        /// If type "const float3 &" -> BasicType() returns "float3".
        /// If type "float3 * const" -> BasicType() returns "float3*".
        /// If type "const float3 * const" -> BasicType() returns "const float3*".
        /// </summary>
        /// <returns></returns>
        public string BasicType()
        {
            string t = type.Trim();
            if (t.EndsWith("&"))
            {
                t = t.Substring(0, t.Length - 1).Trim();
                if (t.StartsWith("const"))
                    t = t.Substring(5).Trim();
            }
            if (t.EndsWith("const"))
                t = t.Substring(0, t.Length - 5).Trim();
            return t;
        }

        /// <summary>
        /// Returns the basic type as a sanitated identifier.
        /// If type "const float3 &" -> BasicType() returns "float3".
        /// If type "const float3 &" -> BasicType() returns "float3".
        /// If type "float3 * const" -> BasicType() returns "float3_ptr".
        /// If type "const float3 * const" -> BasicType() returns "float3_ptr".
        /// </summary>
        /// <returns></returns>
        public string BasicTypeId()
        {
            string t = type.Trim();
            if (t.EndsWith("&"))
                t = t.Substring(0, t.Length - 1).Trim();
            if (t.StartsWith("const"))
                t = t.Substring(5).Trim();
            if (t.EndsWith("const"))
                t = t.Substring(0, t.Length - 5).Trim();
            if (t.EndsWith("*"))
            {
                t = t.Substring(0, t.Length - 1).Trim();

                if (t.EndsWith("const"))
                    t = t.Substring(0, t.Length - 5).Trim();

                t = t + "_ptr";
            }
            return t;
        }
    };

    public class Symbol
    {
        public string kind; // "file", "function", "class", "variable", "struct", "enumvalue", "typedef"
        /// <summary>
        /// If this is a function, specifies the return value.
        /// If this is a member variable, specifies the variable type.
        /// If this is a class or a struct, this field is empty.
        /// </summary>
        public string type;
        public VisibilityLevel visibilityLevel;
        public Virtualness virtualness;
        /// <summary>
        /// The name of the symbol this structure represents.
        /// </summary>
        public string name;
        public string path;
        public string filename;
        public string anchorFile;
        public string anchor;
        public string argList;
        /// <summary>
        /// Specifies under which name this symbol will be present in the class member index.
        /// If empty, this member will be hidden from the index.
        /// </summary>
        public string classMemberIndexTitle;
        public bool isStatic;
        /// <summary>
        /// If this Symbol is "function", this contains a list of all the parameters to the function.
        /// </summary>
        public List<Parameter> parameters = new List<Parameter>();
        public Dictionary<string, Symbol> children = new Dictionary<string,Symbol>();
        public Symbol parent;
        /// <summary>
        /// If non-null, this member specifies an overload of this function that is shown in the
        /// doc webpage instead of this symbol. (This is to make the docs look shorter).
        /// </summary>
        public Symbol similarOverload;
        /// <summary>
        /// This list specifies which other symbols use this symbol as the master doc page.
        /// </summary>
        public List<Symbol> otherOverloads = new List<Symbol>();
        // If kind == "file", this specifies all files this file includes.
        public List<string> includes = new List<string>();
        public List<string> comments = new List<string>();
        /// <summary>
        /// Specifies the documentation for the @return field.
        /// </summary>
        public string returnComment;

        public string BriefComment()
        {
            if (comments.Count == 0)
                return "";
            else
                return comments[0];
        }

        public string category = "";

        public string Namespace()
        {
            int i = name.LastIndexOf("::");
            if (i != -1)
                return name.Substring(0, i);
            return "";
        }

        public Symbol FindChildByName(string name)
        {
            foreach (Symbol s in children.Values)
                if (s.name == name)
                    return s;
            return null;
        }

        public string NameWithoutNamespace()
        {
            string ns = Namespace();
            string nameWithoutNamespace = name.Substring(ns.Length);
            if (nameWithoutNamespace.StartsWith("::"))
                return nameWithoutNamespace.Substring(2);
            else
                return nameWithoutNamespace;
        }

        public bool IsConst()
        {
            if (kind == "function" && argList.EndsWith("const"))
                return true;
            else if (kind == "variable" && type.Contains("const"))
                return true;
            else
                return false;
        }

        public Parameter FindParamByName(string paramName)
        {
            foreach (Parameter p in parameters)
                if (p.name == paramName)
                    return p;
            return null;
        }

        public string Category()
        {
            if (category != "")
                return category;

            foreach (string s in comments)
            {
                int endIdx;
                string categ = CodeStructure.FindStringInBetween(s, 0, "[Category:", "]", out endIdx); 
                if (endIdx != -1)
                {
                    string cat = categ.StripHtmlCharacters();
                    cat = cat.Replace("<br />", "").Replace("<br/>", "").StripHtmlCharacters();
                    if (cat.EndsWith("."))
                        cat = cat.Substring(0, cat.Length - 1).StripHtmlCharacters();
                    return cat;
                }
            }
            return "";
        }

        public string ScopeName()
        {
            string s = "";
            Symbol p = parent;
            while (p != null && (p.kind == "class" || p.kind == "struct"))
            {
                s = p.name + "::" + s;
                p = p.parent;
            }
            return s;
        }

        public string ScopeNameWithoutNamespace()
        {
            string s = "";
            Symbol p = parent;
            while (p != null && (p.kind == "class" || p.kind == "struct"))
            {
                s = p.NameWithoutNamespace() + "::" + s;
                p = p.parent;
            }
            return s;
        }

        public string FullQualifiedSymbolName()
        {
            string s = type + " ";
            s += ScopeName();
            s += name;
            s += argList;
            return s;
        }

        public string FullQualifiedSymbolNameWithoutNamespace()
        {
            string s = type + " ";
            s += ScopeNameWithoutNamespace();
            s += NameWithoutNamespace();
            s += argList;
            return s;
        }

        static string EscapeFilename(string s)
        {
            s = s.Replace(".html", "_")
                .Replace(".", "_")
                .Replace("*", "__mul__")
                .Replace("/", "__div__")
                .Replace("+", "__plus__")
                .Replace("-", "__minus__")
                .Replace("::", "__scope__")
                .Replace(":", "__dc__")
                .Replace("<=", "__leq__")
                .Replace("<", "__lt__")
                .Replace(">=", "__geq__")
                .Replace(">", "__gt__")
                .Replace("==", "__eqa__")
                .Replace("=", "__eq__")
                .Trim();
            return s;
        }

        public string ClassIndexDocFilename()
        {
            return "index_" + EscapeFilename(name) + ".html";
        }

        public string MemberDocumentationFilename()
        {
            if (similarOverload != null)
                return similarOverload.MemberDocumentationFilename();
            return EscapeFilename(anchorFile) + "_" + EscapeFilename(name) + ".html";
        }

        public static bool IsPODType(string type)
        {
            return type == "bool" || type == "int" || type == "float"; ///\todo Add more basic types here.
        }

    };

    public class CodeStructure
    {
        public Dictionary<string, Symbol> symbols;
        public Dictionary<string, Symbol> symbolsByAnchor = new Dictionary<string,Symbol>();
        public SortedSet<string> documentationFiles = new SortedSet<string>();

        /// <summary>
        /// Goes through all symbols recursively and groups similar functions together (sets the similarOverload attribute).
        /// </summary>
        void GroupSimilarOverloads()
        {
            foreach(Symbol s in symbols.Values)
                if (s.kind == "class" || s.kind == "struct")
                    GroupSimilarOverloads(s);
        }

        void GroupSimilarOverloads(Symbol s)
        {
            foreach(Symbol s1 in s.children.Values)
                foreach(Symbol s2 in s.children.Values)
                    if (s1 != s2 && s1.similarOverload == null && s2.similarOverload == null && s2.otherOverloads.Count == 0)
                    {
                        Symbol master = s1;
                        Symbol child = s2;
                        if (master.name == child.name)
                        {
                            if (child.comments.Count == 0)
                            {
                                child.similarOverload = master;
                                master.otherOverloads.Add(child);
                            }
                        }
                    }
        }

        public void LoadSymbolsFromDoxytagFile(string filename, string htmlDocPath)
        {
  //          Console.WriteLine("Loading doxytag xml file \"" + filename + "\"");
            TextReader t = new StreamReader(filename);
            string dataString = t.ReadToEnd();
            XmlDocument doc = new XmlDocument();
            doc.LoadXml(dataString);

            symbols = new Dictionary<string, Symbol>();
            XmlElement root = doc.DocumentElement;
            foreach (XmlNode node in root.ChildNodes)
            {
                XmlElement e = node as XmlElement;
                if (e == null)
                    continue;
                if (e.Name == "compound" && (e.GetAttribute("kind") == "class" || e.GetAttribute("kind") == "struct"))
                    ParseClassCompound(symbols, e);
            }
            foreach (XmlNode node in root.ChildNodes)
            {
                XmlElement e = node as XmlElement;
                if (e == null)
                    continue;
                if (e.Name == "compound" && e.GetAttribute("kind") == "file")
                    ParseFileCompound(symbols, e);
            }

            if (!htmlDocPath.EndsWith("/") && !htmlDocPath.EndsWith("\\"))
                htmlDocPath += "/";
            // Load documentation.
            foreach (string docFile in documentationFiles)
                ParseDocumentationFile(htmlDocPath + docFile);

            GroupSimilarOverloads();
        }

        static private string GetXmlElementChildNodeValue(XmlElement element, string childNode)
        {
            foreach (XmlNode node in element)
            {
                XmlElement e = node as XmlElement;
                if (e == null)
                    continue;
                if (e.Name == childNode)
                    return e.InnerText;
            }
            return "";
        }

        static List<XmlElement> GetChildElementsByName(XmlElement node, string elementName)
        {
            List<XmlElement> elements = new List<XmlElement>();
            foreach (XmlNode n in node.ChildNodes)
            {
                XmlElement e = n as XmlElement;
                if (e == null)
                    continue;
                if (e.Name == elementName)
                    elements.Add(e);
            }
            return elements;
        }

        static private VisibilityLevel ParseVisibilityLevel(XmlElement elem)
        {
            if (!elem.HasAttribute("protection"))
                return VisibilityLevel.Public;
            if (elem.GetAttribute("protection") == "protected")
                return VisibilityLevel.Protected;
            if (elem.GetAttribute("protection") == "private")
                return VisibilityLevel.Private;
            return VisibilityLevel.Public;
        }

        static private Virtualness ParseVirtualness(XmlElement elem)
        {
            if (!elem.HasAttribute("virtualness"))
                return Virtualness.None;
            if (elem.GetAttribute("virtualness") == "virtual")
                return Virtualness.Virtual;
            if (elem.GetAttribute("virtualness") == "pure")
                return Virtualness.Pure;
            return Virtualness.None;
        }

        private void ParseClassCompound(Dictionary<string, Symbol> symbols, XmlElement classNode)
        {
            Symbol newSymbol = new Symbol();
            newSymbol.kind = classNode.GetAttribute("kind").StripHtmlCharacters();
            newSymbol.type = "";
            newSymbol.name = GetXmlElementChildNodeValue(classNode, "name").StripHtmlCharacters();
            newSymbol.path = "";
            newSymbol.filename = GetXmlElementChildNodeValue(classNode, "filename").StripHtmlCharacters();
            newSymbol.visibilityLevel = ParseVisibilityLevel(classNode);
            newSymbol.virtualness = ParseVirtualness(classNode);
            symbols.Add(newSymbol.name, newSymbol);
            List<XmlElement> members = GetChildElementsByName(classNode, "member");
            foreach (XmlElement member in members)
            {
                Symbol child = ParseMemberElement(newSymbol, member);
                string symbolName = child.type + " " + child.name + " " + child.argList;
                while (newSymbol.children.ContainsKey(symbolName))
                    symbolName = symbolName + "_2";

                newSymbol.children.Add(symbolName, child);
            }
        }

        private Symbol ParseMemberElement(Symbol parent, XmlElement memberElem)
        {
            Symbol member = new Symbol();
            member.parent = parent;
            member.kind = memberElem.GetAttribute("kind").StripHtmlCharacters();
            member.isStatic = (memberElem.GetAttribute("static").StripHtmlCharacters() == "yes") ? true : false;
            member.type = GetXmlElementChildNodeValue(memberElem, "type").StripHtmlCharacters();
            if (member.type.StartsWith("static"))
            {
                member.type = member.type.Substring(6).Trim();
                member.isStatic = true;
            }
            member.name = GetXmlElementChildNodeValue(memberElem, "name").StripHtmlCharacters();
            member.classMemberIndexTitle = member.name;
            member.anchorFile = GetXmlElementChildNodeValue(memberElem, "anchorfile").StripHtmlCharacters();
            documentationFiles.Add(member.anchorFile);
            member.anchor = GetXmlElementChildNodeValue(memberElem, "anchor").StripHtmlCharacters();
            member.argList = GetXmlElementChildNodeValue(memberElem, "arglist").StripHtmlCharacters();
            if (member.kind == "function")
                GenerateArgList(member);
            member.visibilityLevel = ParseVisibilityLevel(memberElem);
            member.virtualness = ParseVirtualness(memberElem);
            if (!symbolsByAnchor.ContainsKey(member.anchor))
                symbolsByAnchor.Add(member.anchor, member);

            return member;
        }

        private void ParseFileCompound(Dictionary<string, Symbol> symbols, XmlElement fileNode)
        {
            Symbol newSymbol = new Symbol();
            newSymbol.kind = "file";
            newSymbol.type = "";
            newSymbol.name = GetXmlElementChildNodeValue(fileNode, "name").StripHtmlCharacters();
            newSymbol.path = GetXmlElementChildNodeValue(fileNode, "path").StripHtmlCharacters();
            newSymbol.filename = GetXmlElementChildNodeValue(fileNode, "filename").StripHtmlCharacters();
            newSymbol.includes = new List<string>();
            symbols.Add(newSymbol.name, newSymbol);
            List<XmlElement> includes = GetChildElementsByName(fileNode, "includes");
            foreach (XmlElement include in includes)
            {
                newSymbol.includes.Add(include.InnerText.StripHtmlCharacters());
            }

            List<XmlElement> classes = GetChildElementsByName(fileNode, "class");
            foreach (XmlElement classElem in classes)
            {
                if (symbols.ContainsKey(classElem.InnerText.StripHtmlCharacters()))
                    newSymbol.children.Add(classElem.InnerText, symbols[classElem.InnerText.StripHtmlCharacters()]);
            }
        }

        private string CutDocGeneratorCommentDirective(string s, int startIdx, int endIdx)
        {
            s = s.Remove(startIdx, endIdx);
            if (s.Length > startIdx && s[startIdx] == '.') // Eat the trailing dot, if there was one.
                s = s.Remove(startIdx, 1);
            if (s.Length > startIdx+1 && s[startIdx+1] == '.') // Eat the trailing dot, if there was one.
                s = s.Remove(startIdx+1, 1);
            s = s.Trim();
            return s;
        }

        /// <summary>
        /// Goes through the given symbol and find all DocGenerator-style directives present in the comments
        /// and applies them. These directives are of form [foo].
        /// </summary>
        private void ProcessDocGeneratorCommentDirectives(Symbol s)
        {
            for (int i = 0; i < s.comments.Count; ++i)
            {
                s.comments[i] = s.comments[i].Trim();
                if (s.comments[i].EndsWith("<br/>"))
                    s.comments[i] = s.comments[i].Substring(0, s.comments[i].Length - 5);
                if (s.comments[i].EndsWith("<br />"))
                    s.comments[i] = s.comments[i].Substring(0, s.comments[i].Length - 6);
                s.comments[i] = s.comments[i].Trim();

                int endIdx;
                int startIndex = 0;
                while (startIndex < s.comments[i].Length)
                {
                    string directive = FindStringInBetween(s.comments[i], startIndex, "[", "]", out endIdx);
                    if (endIdx == -1)
                    {
                        s.comments[i] = s.comments[i].Trim();
                        if (s.comments[i].Length == 0)
                        {
                            s.comments.RemoveAt(i);
                            --i;
                        }
                        break;
                    }

                    int directiveStartIndex = endIdx - directive.Length - 1;
                    directive = directive.Trim();
                    string[] st = directive.Split(':');
                    string directiveParam = "";
                    if (st.Length == 2)
                        directiveParam = st[1].Trim();

                    if (directive.ToLower().StartsWith("similaroverload") && directiveParam.Length > 0)
                    {
                        Symbol similarOverloadSymbol = s.parent.FindChildByName(directiveParam);
                        if (similarOverloadSymbol != null)
                        {
                            if (s.similarOverload == null)
                            {
                                s.similarOverload = similarOverloadSymbol;
                                similarOverloadSymbol.otherOverloads.Add(s);
                            }
                        }
                        else
                        {
//                            Console.WriteLine("Can't find similarOverload " + directiveParam + " for member " + s.FullQualifiedSymbolName());
                        }
                        s.comments[i] = CutDocGeneratorCommentDirective(s.comments[i], directiveStartIndex - 1, endIdx + 1 - directiveStartIndex);

                        // Don't update startIndex since we deleted the "[]" block.
                    }
                    else if (directive.ToLower().StartsWith("indextitle") && directiveParam.Length > 0)
                    {
                        s.classMemberIndexTitle = directiveParam;
                        s.comments[i] = CutDocGeneratorCommentDirective(s.comments[i], directiveStartIndex - 1, endIdx + 1 - directiveStartIndex);
                    }
                    else if (directive.ToLower().StartsWith("hideindex"))
                    {
                        s.classMemberIndexTitle = "";
                        s.comments[i] = CutDocGeneratorCommentDirective(s.comments[i], directiveStartIndex - 1, endIdx + 1 - directiveStartIndex);
                    }
                    else
                    {
                        startIndex = endIdx;
                    }
                }
            }
        }

        private static XmlElement FindMemDocSibling(XmlElement e)
        {
            XmlElement sibling = e.NextSibling as XmlElement;
            while (sibling != null && sibling != e)
            {
                if (sibling.Name == "div" && sibling.GetAttribute("class") == "memdoc")
                    return sibling;
                sibling = sibling.NextSibling as XmlElement;
            }
            return null;
        }

        private void FindAnchorElements(XmlElement root, List<XmlElement> anchorElements)
        {
            if (root.Name == "a" && root.GetAttribute("class") == "anchor" && symbolsByAnchor.ContainsKey(root.GetAttribute("id")))
            {
                XmlElement memDocSibling = FindMemDocSibling(root);
                if (memDocSibling != null)
                    anchorElements.Add(root);
            }

            foreach(XmlNode n in root.ChildNodes)
            {
                XmlElement e = n as XmlElement;
                if (e != null)
                    FindAnchorElements(e, anchorElements);
            }
        }

        private static List<string> SplitToParagraphs(string input)
        {
            input = input.Trim();
            List<string> paragraphs = new List<string>();
            int idx = 0;
            int pCount = 0;
            int startOfParagraph = 0;
            while (idx < input.Length)
            {
                int p = input.IndexOf("<p>", idx);
                int closeP = input.IndexOf("</p>", idx);
                if ((p < closeP || p == -1) && p != -1)
                {
                    ++pCount;
                    idx = p + 3;
                    if (pCount == 1)
                        startOfParagraph = idx;
                }
                else if ((closeP < p || p == -1) && closeP != -1)
                {
                    --pCount;
                    idx = closeP + 4;
                    if (pCount == 0)
                        paragraphs.Add(input.Substring(startOfParagraph, closeP - startOfParagraph).Trim());
                    if (pCount < 0)
                        return paragraphs;
                }
                else
                {
//                    if (input.Length > 0)
//                        paragraphs.Add(input);
                    break;
                }
            }
            return paragraphs;
        }

        private bool FindMemdocContent(string dataString, int startIndex, out string memdocString, out int memdocEnd)
        {
            string memdoc = "<div class=\"memdoc\">";
            int memdocStartIndex = dataString.IndexOf(memdoc, startIndex);
            if (memdocStartIndex == -1)
            {
                memdocString = "";
                memdocEnd = startIndex;
                return false;
            }
            int nextDoxytagStartIndex = dataString.IndexOf("doxytag:", startIndex);
            if (nextDoxytagStartIndex != -1 && nextDoxytagStartIndex < memdocStartIndex)
            {
                memdocString = "";
                memdocEnd = startIndex;
                return false;
            }
            memdocStartIndex += memdoc.Length;
            startIndex = memdocStartIndex;

            string memdocEndTag = "</div>";
            memdocEnd = dataString.IndexOf(memdocEndTag, startIndex);
            if (memdocEnd == -1)
            {
                memdocString = "";
                memdocEnd = startIndex;
                return false;
            }

            memdocString = dataString.Substring(memdocStartIndex, memdocEnd - memdocStartIndex);
            return true;
        }

        private bool FindBriefdocContent(string dataString, int startIndex, out string memdocString, out int memdocEnd)
        {
            string memdoc = "<td class=\"mdescRight\">";
            int memdocStartIndex = dataString.IndexOf(memdoc, startIndex);
            if (memdocStartIndex == -1)
            {
                memdocString = "";
                memdocEnd = startIndex;
                return false;
            }
            int nextDoxytagStartIndex = dataString.IndexOf("doxytag:", startIndex);
            if (nextDoxytagStartIndex != -1 && nextDoxytagStartIndex < memdocStartIndex)
            {
                memdocString = "";
                memdocEnd = startIndex;
                return false;
            }
            memdocStartIndex += memdoc.Length;
            startIndex = memdocStartIndex;

            string memdocEndTag = "</td>";
            memdocEnd = dataString.IndexOf(memdocEndTag, startIndex);
            if (memdocEnd == -1)
            {
                memdocString = "";
                memdocEnd = startIndex;
                return false;
            }

            memdocString = dataString.Substring(memdocStartIndex, memdocEnd - memdocStartIndex);
            return true;
        }

        /// <summary>
        /// Outputs endIdx = -1 if not found.
        /// </summary>
        /// <param name="dataString"></param>
        /// <param name="startIdx"></param>
        /// <param name="start"></param>
        /// <param name="end"></param>
        /// <param name="endIdx"></param>
        /// <returns></returns>
        public static string FindStringInBetween(string dataString, int startIdx, string start, string end, out int endIdx)
        {
            endIdx = -1;
            int patternStart = dataString.IndexOf(start, startIdx);
            if (patternStart == -1)
                return "";
            patternStart += start.Length;
            int patternEnd = dataString.IndexOf(end, patternStart);
            if (patternEnd == -1)
                return "";
            string text = dataString.Substring(patternStart, patternEnd - patternStart);
            endIdx = patternEnd + end.Length;
            return text;
        }

        /// <summary>
        /// Each doxygen documentation file on a class contains a single class comment near the top of the page.
        /// </summary>
        /// <param name="dataString"></param>
        private void ParseDocumentationFileForClassComments(string dataString)
        {
            int endIndex;
            string className = FindStringInBetween(dataString, 0, "<!-- doxytag: class=\"", "\" -->", out endIndex);
            if (endIndex == -1)
                return;
            if (!symbols.ContainsKey(className))
                return;
            Symbol classSymbol = symbols[className];

            string classComments = FindStringInBetween(dataString, endIndex,
                "<hr/><a name=\"_details\"></a><h2>Detailed Description</h2>",
                "<hr/><h2>Member",
                out endIndex);
            if (endIndex != -1)
            {
                classSymbol.comments.AddRange(SplitToParagraphs(classComments));
            }
        }

        /// <summary>
        /// Breaks down the single string form arglist into a member args list structure.
        /// </summary>
        /// <param name="s"></param>
        static private void GenerateArgList(Symbol s)
        {
            s.parameters.Clear();
            string argList = s.argList;
            if (argList.StartsWith("("))
                argList = argList.Substring(1);
            if (argList.EndsWith(")"))
                argList = argList.Substring(0, argList.Length - 1);
            if (argList.EndsWith(")=0"))
                argList = argList.Substring(0, argList.Length - 3);
            argList = argList.Trim();
            if (argList.Length == 0)
                return;
            string[] args = argList.Split(',');
            foreach (string arg in args)
            {
                string a = arg.Trim();
//                int lastSpace = a.LastIndexOf(' ');
                Parameter p = new Parameter();
                p.type = a.StripHtmlCharacters().StripHtmlLinks();// a.Substring(0, lastSpace).Trim();
//                p.name = a.Substring(lastSpace + 1, a.Length - (lastSpace + 1)).Trim();
                s.parameters.Add(p);
            }
        }

        private void ParseFunctionArgList(Symbol s, string content)
        {
            s.parameters.Clear();

            int endIdx;
            content = FindStringInBetween(content, 0, "<table class=\"memname\">", "</table>", out endIdx);
            if (endIdx == -1)
                return;

            int startIdx = 0;
            while (startIdx < content.Length)
            {
                string paramType = FindStringInBetween(content, startIdx, "<td class=\"paramtype\">", "</td>", out endIdx);
                if (endIdx == -1)
                    return;
                startIdx = endIdx+1;
                string paramName = FindStringInBetween(content, startIdx, "<td class=\"paramname\">", "</td>", out endIdx);
                if (endIdx == -1)
                    return;
                startIdx = endIdx + 1;
                string strippedParamName = FindStringInBetween(paramName, 0, "<em>", "</em>", out endIdx);

                Parameter p = new Parameter();
                p.type = paramType.StripHtmlCharacters().StripHtmlLinks();
                p.name = strippedParamName.StripHtmlCharacters().StripHtmlLinks();
                s.parameters.Add(p);
            }
        }

        private void ParseFunctionParameterComments(Symbol s, string memdocContent)
        {
            int endIdx;
            string memberData = FindStringInBetween(memdocContent, 0, "<dl><dt><b>Parameters:</b></dt><dd>",
                "</table>", out endIdx);
            if (endIdx == -1)
                return;
            int startIdx = 0;
            while(startIdx < memberData.Length)
            {
                string row = FindStringInBetween(memberData, startIdx, "<tr>", "</tr>", out endIdx);
                if (endIdx == -1)
                    return;
                startIdx = endIdx+1;
                int paramNameEnd;
                string paramName = FindStringInBetween(row, 0, "<em>", "</em>", out paramNameEnd);
                paramName = paramName.Trim();
                if (paramNameEnd == -1)
                    continue;
                int cmEnd;
                string comment = FindStringInBetween(row, paramNameEnd, "<td>", "</td>", out cmEnd);
                if (cmEnd == -1)
                    continue;
                Parameter param = s.FindParamByName(paramName);
                if (param == null)
                    continue;

                param.comment = comment.Trim();
            }
        }

        private void ParseFunctionReturnValueComments(Symbol s, string memdocContent)
        {
            int endIdx;
            string memberData = FindStringInBetween(memdocContent, 0, "<dl class=\"return\"><dt><b>Returns:</b></dt><dd>",
                "</dd></dl>", out endIdx);
            if (endIdx == -1)
                return;
            s.returnComment = memberData;
        }

        private void ParseDocumentationFileForBriefMemberComments(string dataString)
        {
            int startIdx = 0;
            while(startIdx < dataString.Length)
            {
                int endIdx;
                string elemRefStr = "<tr><td class=\"memItemLeft\" align=\"right\" valign=\"top\">";
                string memberLine1 = FindStringInBetween(dataString, startIdx,
                    elemRefStr,
                    "</tr>", out endIdx);
                if (endIdx == -1)
                    return;
                startIdx = endIdx + 1;
                int endIdx2;
                int nextElemRefEnd;
                string nextElemRef = FindStringInBetween(dataString, endIdx+1,
                    elemRefStr, "</tr>", out nextElemRefEnd);
                string memberLine2 = FindStringInBetween(dataString, endIdx + 1,
                    "<tr><td class=\"mdescLeft\">", "</tr>",
                    out endIdx2);
                if (endIdx2 == -1)
                    continue;
                if (endIdx2 >= nextElemRefEnd)
                    continue; // Parsed too far with the comment line.
                startIdx = endIdx2 + 1;

                // Find the ref this brief comment is for.
                string refString = FindStringInBetween(memberLine1, 0, " ref=\"", "\"", out endIdx);
                if (endIdx == -1)
                    continue;
                if (!symbolsByAnchor.ContainsKey(refString))
                    continue;
                Symbol s = symbolsByAnchor[refString];
                string briefComment = FindStringInBetween(memberLine2, 0, "<td class=\"mdescRight\">",
                    "</td>", out endIdx);
                if (endIdx == -1)
                    continue;
                s.comments.Add(briefComment);
            }
        }

        private void ParseDocumentationFile(string filename)
        {
//            Console.WriteLine("Parsing documentation from file \"" + filename + "\"");
            TextReader t = new StreamReader(filename);
            string dataString = t.ReadToEnd();

            ParseDocumentationFileForClassComments(dataString);
            ParseDocumentationFileForBriefMemberComments(dataString);
            int startIndex = 0;
            int strLen = dataString.Length;
            while (startIndex < strLen)
            {
                int index = dataString.IndexOf("doxytag:", startIndex);
                if (index == -1)
                    break;
                startIndex = index + 1;
                int refStartIndex = dataString.IndexOf(" ref=\"", startIndex);
                if (refStartIndex == -1)
                    break;
                int refIndex = refStartIndex + 6;
                int refEndIndex = dataString.IndexOf("\"", refIndex);
                if (refEndIndex == -1)
                    break;
                startIndex = refEndIndex + 1;
                string anchorRef = dataString.Substring(refIndex, refEndIndex - refIndex);
                if (!symbolsByAnchor.ContainsKey(anchorRef))
                    continue;
                Symbol s = symbolsByAnchor[anchorRef];

                // Try to find memdoc content (detailed comment block).
                int memdocEnd;
                string memdocContent;
                bool success = FindMemdocContent(dataString, startIndex, out memdocContent, out memdocEnd);
                if (success)
                {
                    s.comments.AddRange(SplitToParagraphs(memdocContent));

                    if (s.kind == "function")
                    {
                        int protoEndIdx;
                        string memProto = FindStringInBetween(dataString, startIndex, "<div class=\"memproto\">",
                            "</div>", out protoEndIdx);
                        if (protoEndIdx != -1)
                            ParseFunctionArgList(s, memProto);
                        ParseFunctionParameterComments(s, memdocContent);
                        ParseFunctionReturnValueComments(s, memdocContent);
                    }

                    startIndex = memdocEnd + 1;
                }
                else
                {
                    success = FindBriefdocContent(dataString, startIndex, out memdocContent, out memdocEnd);
                    if (success)
                    {
                        s.comments.AddRange(SplitToParagraphs(memdocContent));
                        startIndex = memdocEnd + 1;
                    }
                }
            }

            foreach (Symbol s in symbolsByAnchor.Values)
                ProcessDocGeneratorCommentDirectives(s);

//            XmlDocument doc = new XmlDocument();
 //           doc.XmlResolver = null;
  //          doc.LoadXml(dataString);
/*
            HTMLDocument doc = new HTMLDocument();
            IHTMLDocument2 doc2 = (IHTMLDocument2)doc;
            doc2.write(dataString);
            doc.get
*/
   //         WebBrowser wb = new WebBrowser();
  //          wb.DocumentText = dataString;
 //           HtmlDocument doc = wb.Document;// new HtmlDocument();
//            doc.
//            doc.
/*
            List<XmlElement> anchorElements = new List<XmlElement>();
            FindAnchorElements(doc.DocumentElement, anchorElements);
            foreach (XmlElement n in anchorElements)
            {
                XmlElement memDocSibling = FindMemDocSibling(n);
                Symbol s = symbolsByAnchor[memDocSibling.GetAttribute("id")];

                List<XmlElement> paragraphs = GetChildElementsByName(memDocSibling, "p");
                foreach (XmlElement p in paragraphs)
                {
                    s.comments.Add(p.InnerXml);
                }
            }
 */ 
        }
    }

    static public class StringExtensions
    {
        public static string StripHtmlCharacters(this string str)
        {            
            return System.Net.WebUtility.HtmlDecode(str.Replace("&nbsp;", " ").Replace("&#160;", " ").Replace("&amp;", "&")).Trim();
        }
        public static string StripHtmlLinks(this string str)
        {
            for(;;)
            { ///\todo regex.
                int start = str.IndexOf("<a");
                if (start == -1)
                    start = str.IndexOf("</a");
                if (start == -1)
                    return str;
                int end = str.IndexOf(">");
                if (end == -1)
                    return str;
                str = str.Substring(0, start) + str.Substring(end + 1);
            }
        }
    }

}
