# Software #
apache: apache 2.2.22 stable<br>
nginx: nginx 1.0.15<br>
iis: iis 7.0<br>
fiber: icyhttpd - fiberio<br>
thread: icyhttpd - naiveio<br>
<br>
<h1>Script</h1>

<pre><code>Measure-Command { tinyget -srv:127.0.0.1 -r:80 -uri:/file.txt -threads:4 -loop:1024 }<br>
</code></pre>

<h1>Result</h1>

in time(sec), the lower the better<br>
<br>
<table><thead><th> </th><th> 256 </th><th> 4096 </th><th> 65536 </th><th> 1048576 </th></thead><tbody>
<tr><td> apache </td><td> 0.602 </td><td> 0.600 </td><td> 0.734 </td><td> 5.174 </td></tr>
<tr><td> nginx </td><td> 0.856 </td><td> 0.932 </td><td> 1.159 </td><td> 6.811 </td></tr>
<tr><td> iis </td><td> 0.421 </td><td> 0.423 </td><td> 0.624 </td><td> 5.267 </td></tr>
<tr><td> fiber </td><td> 0.514 </td><td> 0.520 </td><td> 0.632 </td><td> 4.942 </td></tr>
<tr><td> thread </td><td> 0.587 </td><td> 0.591 </td><td> 0.687 </td><td> 4.954 </td></tr>