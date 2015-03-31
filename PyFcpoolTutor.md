# Installation #

  * Windows only
  * Precompiled binary is for Python 2.7
  * Single file ([fcpool.pyd](http://code.google.com/p/icyhttpd/downloads/list)), copy anywhere
  * Currently the repository is [here](http://iceboy.0ginr.com/svn/fcpool)

# Example #

Assume that:
  * Python locates at C:\Python27
  * PHP locates at C:\php\php-cgi.exe
  * PHP code stores in D:\www\test.php

Copy fcpool.pyd into C:\Python27\DLLs

PHP code:

```
<?
print $_POST['hello']
?>
```

Python code:

```
from fcpool import *
p = Pool("c:\\php\\php-cgi.exe")
r = Request(p, {
	"SCRIPT_FILENAME": "d:\\www\\test.php",
	"REQUEST_METHOD": "POST",
	"CONTENT_LENGTH": "11",
	"CONTENT_TYPE": "application/x-www-form-urlencoded"
})
r.write('hello=world')
r.write(None)
while True:
	b = r.read(4096)
	print(b)
	if len(b) < 4096:
		break
```

Result:

```
X-Powered-By: PHP/5.3.8
Content-type: text/html

world
```

# Semantics #

Module **fcpool** contains two classes: **Pool** and **Request**

**Pool**(cmd\_line, queue\_length = 0, max\_instances = 0, idle\_time = 30000, max\_requests = 499)
  * cmd\_line: the command line to create new instance
  * queue\_length: maximum length of the queue containing pending requests (zero means no limit)
  * max\_instances: maximum simutaneously running instances (zero means no limit)
  * idle\_time: time (in milliseconds) to pool an instance before killing it
  * max\_requests: maximum requests an instance can handle, typically 499 for php-cgi ([why?](http://mailman.nginx.org/pipermail/nginx/2008-March/004064.html))

**Request**(pool, params)
  * pool: the pool object
  * params: a dict object which will be transferred into FastCGI param stream

Notice that creating a request object actually begins it, therefore may pend the thread

**Request** has two methods: read and write

  * buffer = **Request**.read(_buffer size_)
  * size = **Request**.write(_buffer_)

buffer is actually a string. If len(buffer) < _buffer size_, end-of-file is reached. For example if the total response is 500 bytes, use 250 as _buffer size_ will receive buffers size 250, 250, 0; use 256 as _buffer size_ will receive 256, 244. write(None) indicates end-of-file, and the return value is 0; write(_empty string_) does nothing.

Multithread is supported: the implementation of the **fcpool** module is fully asynchronous and no thread is involved, the python module wraps it in a synchronous way.

# HTTP Server Example #

Simply modifying the CGIHttpServer.py comes FcHttpServer:

```
import os
import urllib
import SimpleHTTPServer
import SocketServer
import fcpool

pool = fcpool.Pool("c:\\php\\php-cgi.exe")

class FcHttpRequestHandler(SimpleHTTPServer.SimpleHTTPRequestHandler):
    def do_POST(self):
        pool = self.get_pool()
        if pool:
            self.run_cgi(pool)
        else:
            self.send_error(501, "Can only POST to CGI scripts")

    def send_head(self):
        pool = self.get_pool()
        if pool:
            return self.run_cgi(pool)
        else:
            return SimpleHTTPServer.SimpleHTTPRequestHandler.send_head(self)

    def get_pool(self):
        # identify file type here. returning None for static files,
        # returning different Pool objects to use different FastCGI pool
        return pool;

    def run_cgi(self, pool):
        """Execute a CGI script."""
        path = self.path
        dir, rest = _url_collapse_path_split(path)

        if dir == '/':
            dir = ''
            
        i = path.find('/', len(dir) + 1)
        while i >= 0:
            nextdir = path[:i]
            nextrest = path[i+1:]

            scriptdir = self.translate_path(nextdir)
            if os.path.isdir(scriptdir):
                dir, rest = nextdir, nextrest
                i = path.find('/', len(dir) + 1)
            else:
                break

        # find an explicit query string, if present.
        i = rest.rfind('?')
        if i >= 0:
            rest, query = rest[:i], rest[i+1:]
        else:
            query = ''

        # dissect the part after the directory name into a script name &
        # a possible additional path, to be stored in PATH_INFO.
        i = rest.find('/')
        if i >= 0:
            script, rest = rest[:i], rest[i:]
        else:
            script, rest = rest, ''

        scriptname = dir + '/' + script
        scriptfile = self.translate_path(scriptname)

        # Reference: http://hoohoo.ncsa.uiuc.edu/cgi/env.html
        # XXX Much of the following could be prepared ahead of time!
        env = dict(os.environ)
        env['SERVER_SOFTWARE'] = self.version_string()
        #env['SERVER_NAME'] = self.server.server_name
        env['GATEWAY_INTERFACE'] = 'CGI/1.1'
        env['SERVER_PROTOCOL'] = self.protocol_version
        #env['SERVER_PORT'] = str(self.server.server_port)
        env['REQUEST_METHOD'] = self.command
        uqrest = urllib.unquote(rest)
        env['PATH_INFO'] = uqrest
        env['PATH_TRANSLATED'] = self.translate_path(uqrest)
        env['SCRIPT_NAME'] = scriptname
        env['SCRIPT_FILENAME'] = scriptfile
        if query:
            env['QUERY_STRING'] = query
        host = self.address_string()
        if host != self.client_address[0]:
            env['REMOTE_HOST'] = host
        env['REMOTE_ADDR'] = self.client_address[0]
        authorization = self.headers.getheader("authorization")
        if authorization:
            authorization = authorization.split()
            if len(authorization) == 2:
                import base64, binascii
                env['AUTH_TYPE'] = authorization[0]
                if authorization[0].lower() == "basic":
                    try:
                        authorization = base64.decodestring(authorization[1])
                    except binascii.Error:
                        pass
                    else:
                        authorization = authorization.split(':')
                        if len(authorization) == 2:
                            env['REMOTE_USER'] = authorization[0]
        # XXX REMOTE_IDENT
        if self.headers.typeheader is None:
            env['CONTENT_TYPE'] = self.headers.type
        else:
            env['CONTENT_TYPE'] = self.headers.typeheader
        length = self.headers.getheader('content-length')
        if length:
            env['CONTENT_LENGTH'] = length
        referer = self.headers.getheader('referer')
        if referer:
            env['HTTP_REFERER'] = referer
        accept = []
        for line in self.headers.getallmatchingheaders('accept'):
            if line[:1] in "\t\n\r ":
                accept.append(line.strip())
            else:
                accept = accept + line[7:].split(',')
        env['HTTP_ACCEPT'] = ','.join(accept)
        ua = self.headers.getheader('user-agent')
        if ua:
            env['HTTP_USER_AGENT'] = ua
        co = filter(None, self.headers.getheaders('cookie'))
        if co:
            env['HTTP_COOKIE'] = ', '.join(co)
        # XXX Other HTTP_* headers
        # Since we're setting the env in the parent, provide empty
        # values to override previously set values
        for k in ('QUERY_STRING', 'REMOTE_HOST', 'CONTENT_LENGTH',
                  'HTTP_USER_AGENT', 'HTTP_COOKIE', 'HTTP_REFERER'):
            env.setdefault(k, "")

        self.send_response(200, "Script output follows")

        decoded_query = query.replace('+', ' ')

        try:
            nbytes = int(length)
        except (TypeError, ValueError):
            nbytes = 0

        if self.command.lower() == "post" and nbytes > 0:
            data = self.rfile.read(nbytes)
        else:
            data = None

        req = fcpool.Request(pool, env)
        req.write(data)

        while True:
            b = req.read(4096)
            self.wfile.write(b)
            if (len(b) < 4096):
                break

# TODO(gregory.p.smith): Move this into an appropriate library.
def _url_collapse_path_split(path):
    """
    Given a URL path, remove extra '/'s and '.' path elements and collapse
    any '..' references.

    Implements something akin to RFC-2396 5.2 step 6 to parse relative paths.

    Returns: A tuple of (head, tail) where tail is everything after the final /
    and head is everything before it.  Head will always start with a '/' and,
    if it contains anything else, never have a trailing '/'.

    Raises: IndexError if too many '..' occur within the path.
    """
    # Similar to os.path.split(os.path.normpath(path)) but specific to URL
    # path semantics rather than local operating system semantics.
    path_parts = []
    for part in path.split('/'):
        if part == '.':
            path_parts.append('')
        else:
            path_parts.append(part)
    # Filter out blank non trailing parts before consuming the '..'.
    path_parts = [part for part in path_parts[:-1] if part] + path_parts[-1:]
    if path_parts:
        tail_part = path_parts.pop()
    else:
        tail_part = ''
    head_parts = []
    for part in path_parts:
        if part == '..':
            head_parts.pop()
        else:
            head_parts.append(part)
    if tail_part and tail_part == '..':
        head_parts.pop()
        tail_part = ''
    return ('/' + '/'.join(head_parts), tail_part)

if __name__ == '__main__':
    SocketServer.ThreadingTCPServer(("", 1225), FcHttpRequestHandler).serve_forever()
```