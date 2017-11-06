function v = converged()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 5);
  end
  v = vInitialized;
end
