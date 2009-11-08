from turbogears import config, expose
import model
import user as srusers

class Admin(object):
	@expose("json")
	@srusers.require(srusers.in_team(), srusers.is_ide_admin)
	def teamname(self, id, name):
		"""
		Change the name of the team with id=id to the passed name
		"""
		try:
			team = model.TeamNames.get(id)
			team.name = name
		except model.SQLObjectNotFound:
			team = model.TeamNames(id=id, name=name)
		team.set()
		return dict(success=1, id=id, name=name)

	@expose("json")
	@srusers.require(srusers.in_team())
	def listblogfeeds(self):
		"""
		List all the blog feeds in the DB
		"""
		feeds = model.UserBlogFeeds.selectBy()
		return dict(feeds=list(feeds))

	@expose("json")
	@srusers.require(srusers.in_team(), srusers.is_ide_admin)
	def setfeedstatus(self, id, url, status):
		"""
		Change the status of a particular feed
		"""
		feeds = model.UserBlogFeeds.selectBy(id=id, url=url)
		try:
			feed = feeds.getOne()
			if status == 'valid':
				feed.checked = True
				feed.valid = True
			elif status == 'invalid':
				feed.checked = True
				feed.valid = False
			elif status == 'unchecked':
				feed.checked = False
			feed.set()
		except:
			return dict(success=0)

		return dict(success=1, id=id, url=url, status=status)

