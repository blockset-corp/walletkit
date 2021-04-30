import org.jetbrains.kotlin.gradle.tasks.KotlinCompile

plugins {
    kotlin("jvm") version "1.4.20"
}

apply(from = "../gradle/publish.gradle")

val deps: Map<*, *> by rootProject.extra

kotlin {
    dependencies {
        api(project(":WalletKit"))
        implementation(deps["coroutinesCore"]!!)
    }
}

tasks.withType<KotlinCompile> {
    kotlinOptions {
        jvmTarget = "1.8"
    }
}
