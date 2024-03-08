from conan import ConanFile
from conan.tools.build import check_min_cppstd
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps
from conan.tools.files import update_conandata
from conan.tools.scm import Git


class ReactAIORecipe(ConanFile):
	name = 'ReactAIO'
	description = 'An all-in-one discord bot written in C++ using the D++ library.'
	version = '1.0'
	license = 'MIT'
	topics = ('Discord', 'Discord bot', 'Moderation')
	url = 'https://github.com/AA1999/Reactaio/'
	settings = 'os', 'compiler', 'build_type', 'arch'
	generators = 'CMakeDeps', 'CMakeToolchain'

	options = {
		'shared': [True, False],
		'fPIC': [True, False],
		'coroutines': [True, False],
		'modules': [True, False]
	}
	default_options = {
		'shared': False,
		'fPIC': True,
		'coroutines': False,
		'modules': False
	}
	exports_sources = 'CMakeLists.txt', 'src/*'

	def requirements(self):
		self.requires('nlohmann_json/3.11.2')
		self.requires('spdlog/1.13.0')
		self.requires('libpqxx/7.9.0')

	def validate(self):
		if self.options.modules:
			check_min_cppstd(self, '23')
		else:
			check_min_cppstd(self, '20')

	def export(self):
		git = Git(self, self.recipe_folder)
		scm_url, scm_commit = git.get_url_and_commit()
		self.output.info(f'Obtained URL: {scm_url} and {scm_commit}.')
		update_conandata(self, {'sources': {'commit': scm_commit, 'url': scm_url}})

	def source(self):
		git = Git(self)
		sources = self.conan_data['sources']
		self.output.info(f'Cloning git repository... {sources}')
		git.clone(url=sources['url'], target='.')
		git.checkout(commit=sources['commit'])

	def config_options(self):
		if self.settings.os == 'Windows':
			self.options.rm_safe('fPIC')

	def layout(self):
		cmake_layout(self)

	def generate(self):
		deps = CMakeDeps(self)
		deps.generate()
		tc = CMakeToolchain(self)
		tc.generate()

	def build(self):
		cmake = CMake(self)
		cmake.configure()
		cmake.build()


