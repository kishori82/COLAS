package daemons

import (
	"fmt"
	"github.com/gorilla/mux"
	"net/http"
)

func HTTP_Server() {
	// Setup HTTP functionality
	router := mux.NewRouter()
	fmt.Println("Running http server")

	router.HandleFunc("/SetAlgorithm/{id}", SetAlgorithm)

	router.HandleFunc("/SetRunId/{id}", SetRunId)
	router.HandleFunc("/SetWriteTo/{param}", SetWriteTo)

	router.HandleFunc("/GetName", GetName)
	router.HandleFunc("/SetName/{param}", SetName)

	router.HandleFunc("/GetLogs", GetLogs)
	router.HandleFunc("/FlushLogs", FlushLogs)

	router.HandleFunc("/GetLog", GetLog)
	router.HandleFunc("/FlushLog", FlushLog)

	router.HandleFunc("/StopAProcess/{ip}", StopAProcess)
	router.HandleFunc("/StartAProcess/{ip}", StartAProcess)
	router.HandleFunc("/KillAProcess/{ip}", KillAProcess)

	router.HandleFunc("/StopProcess", StopProcess)
	router.HandleFunc("/StartProcess", StartProcess)

	router.HandleFunc("/StopReaders", StopReaders)
	router.HandleFunc("/StopWriters", StopWriters)
	router.HandleFunc("/StopServers", StopServers)

	router.HandleFunc("/StartReaders", StartReaders)
	router.HandleFunc("/StartWriters", StartWriters)
	router.HandleFunc("/StartServers", StartServers)

	router.HandleFunc("/KillSelf", KillSelf)

	router.HandleFunc("/SetReaders/{ip:[0-9._]+}", SetReaders)
	router.HandleFunc("/SetWriters/{ip:[0-9._]+}", SetWriters)
	router.HandleFunc("/SetServers/{ip:[0-9._]+}", SetServers)

	router.HandleFunc("/GetReaders", GetReaders)
	router.HandleFunc("/GetWriters", GetWriters)
	router.HandleFunc("/GetServers", GetServers)

	router.HandleFunc("/SetSeed/{seed:[0-9]+}", SetSeed)
	router.HandleFunc("/GetSeed", GetSeed)
	router.HandleFunc("/GetParams", GetParams)

	router.HandleFunc("/SetReadRateDistribution/{param:[a-zA-Z0-9._]+}", SetReadRateDistribution)
	router.HandleFunc("/SetWriteRateDistribution/{param:[a-zA-Z0-9._]+}", SetWriteRateDistribution)

	router.HandleFunc("/SetFileSize/{size:[0-9.]+}", SetFileSize)
	router.HandleFunc("/GetFileSize", GetFileSize)
	http.ListenAndServe(":8080", router)
}
