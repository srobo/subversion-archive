from turbogears import config, expose
import model
import user as srusers

class Admin(object):

	user = srusers.User()

	@expose("json")
	@srusers.require(srusers.in_team())
	def teamname(self, id, name):
		try:
			team = model.TeamNames.get(id)
			team.name = name
		except model.SQLObjectNotFound:
			team = model.TeamNames(id=id, name=name)
		team.set()
		return dict(success=1, id=id, name=name)

