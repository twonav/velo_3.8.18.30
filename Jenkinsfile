pipeline {
  agent {
    label 'linux'
  }
  stages {
    stage('Check') {
      steps {
        sh 'ls -lha'
	sh 'tree'
	sh 'ls -lha ../'
      }
    }
  }
}
