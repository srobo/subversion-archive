from turbogears import config, expose
import feedparser

sr_message_feed = "http://sr2010messages.blogspot.com/feeds/posts/default?alt=rss"

class SRMessageFeed():
	def __init__(self):
		self.fd = feedparser.parse(sr_message_feed)	

	def GetMessages(self):
		msglist = []
		for e in self.fd.entries:
			msglist.append({"title":e.title, "link":e.link, "author":e.author_detail.name,
					"date":e.date, "body":e.description[0:100]})
		return dict(msgs=msglist)

# Single instance of the message feed shared by all users
sfd =  SRMessageFeed()	

class Switchboard(object):
	@expose("json")
	def getmessages(self):
		return sfd.GetMessages()

	@expose("json")
	def timeline(self):
		
		return dict(progress=90, dates=[{"date":"0", "title":"kickstart", "desc":"start"}, 
					{"date":"20", "title":"competition", "desc":"middle"},
					{"date":"100", "title":"pub", "desc":"end"}])
