import os

defines = {}
filenames = os.listdir("sites")
for filename in filenames:
    if filename.endswith(".html"):
        status = filename.split('.')[0]
        with open(os.path.join("sites",filename), 'r') as f:
            content = f.read()
            content = content.replace("    ", "")
            l = len(content);
            content = content.replace('"', '\\"')
            content = content.replace('\n', '\\n')
            header = "HTTP/1.1 %s\\r\\n" % status.replace('_', ' ')
            header+= "Content-Type: text/html\\r\\n"
            header+= "Content-Length: %d\\r\\n" % l
            header+= "Connection: close\\r\\n"
            http = header + "\\r\\n" + content
            defines["HTTP_CODE_"+status.upper()] = http

with open("http.h", "w") as f:
    print >>f, "#ifndef HTTP_H"
    print >>f, "#define HTTP_H"
    for macro in sorted(defines.keys()):
        print >>f, ""
        t = macro.split('_',3)
        print >>f, "/* Status %s: %s */" % (t[2], t[3].title())
        print >>f, "#define %s \"%s\";" % (macro, defines[macro])
    print >>f, "#endif"
