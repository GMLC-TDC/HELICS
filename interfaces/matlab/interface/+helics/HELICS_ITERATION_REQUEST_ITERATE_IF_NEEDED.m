function v = HELICS_ITERATION_REQUEST_ITERATE_IF_NEEDED()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 134);
  end
  v = vInitialized;
end
