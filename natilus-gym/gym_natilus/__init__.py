from gym.envs.registration import register

register(
	id='natilus-v0',
	entry_point='gym_natilus.envs:NatilusEnv'
)
