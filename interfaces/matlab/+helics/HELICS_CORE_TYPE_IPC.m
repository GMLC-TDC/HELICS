function v = helics_core_type_ipc()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1946183037);
  end
  v = vInitialized;
end
