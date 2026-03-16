pipeline {
  agent any

  stages {
    stage('Checkout') {
      steps {
        checkout scm
      }
    }

    stage('Build') {
      steps {
        echo 'Running build step...'
      }
    }

    stage('Test') {
      steps {
        echo 'Running test step...'
      }
    }
  }

  post {
    always {
      echo 'Pipeline finished.'
    }
    success {
      echo 'Pipeline succeeded.'
    }
    failure {
      echo 'Pipeline failed.'
    }
  }
}
