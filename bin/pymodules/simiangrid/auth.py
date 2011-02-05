#httplib was ok and httplib2 especially had nice api, but they don't work thru proxies and stuff
#-- curl is the most robust thing
#import httplib
import curl #a high level wrapper over pycurl bindings
import json
import md5 #only 'cause has a hardcoded pwd here now - for real this comes from connection or launcher

url = "http://localhost/Grid/"
user = "Test User"
pwd = "tester"
md5hex = md5.new(pwd).hexdigest()

pdict = {'RequestMethod': 'AuthorizeIdentity', 
         'Identifier': user, 
         'Type': 'md5hash', 
         'Credential': md5hex}

c = curl.Curl()
rdata = c.post(url, pdict) #does urlencode in the wrapper

print rdata
r = json.loads(rdata)

#http://code.google.com/p/openmetaverse/wiki/AuthorizeIdentity
success = r.get('Success') 
#NOTE: docs say reply should have Success:false upon failure.
#however in my test run it doesn't just the Message of missing/invalid creds
#this code works for that too. 

print "Authentication success:", success


