function v = HELICS_CORE_TYPE_MPI()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1329876586);
  end
  v = vInitialized;
end
