function v = helics_core_type_ipc()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 5);
  end
  v = vInitialized;
end
